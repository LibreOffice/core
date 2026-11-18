/* -*- js-indent-level: 8; fill-column: 100 -*- */

/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * This file contains service which will coordinate intensive DOM modifications
 * which may impact browser rendering performance.
 */

type TaskId = string;
type SimpleTask = () => void;
type LayoutingTask = { taskId: TaskId; func: SimpleTask };

class LayoutingService {
	private _requestedFrame: ReturnType<typeof requestAnimationFrame> | null =
		null;
	private _layoutTasks: Array<LayoutingTask> = [];
	private _layoutTaskFlush: ReturnType<typeof setTimeout> | null = null;
	private _drainCallbacks: Array<SimpleTask> = [];
	private _lastTaskId = 0;

	// get something around 25 fps as minimum (35ms + some overflow = ~40ms)
	private MAX_TASK_DURATION_MS = 35;
	private MIN_TIMER_DELAY_MS = 10;

	public appendLayoutingTask(task: SimpleTask): TaskId {
		const taskObject = {
			taskId: '' + this._lastTaskId++,
			func: task,
		} as LayoutingTask;
		this._layoutTasks.push(taskObject);
		this._scheduleLayouting();
		return taskObject.taskId;
	}

	public cancelLayoutingTask(id: TaskId): void {
		this._layoutTasks = this._layoutTasks.filter((task: LayoutingTask) => {
			return task.taskId !== id;
		});
	}

	public hasTasksPending(): boolean {
		return this._layoutTasks.length > 0;
	}

	public onDrain(callback: SimpleTask): void {
		this._drainCallbacks.push(callback);
		this._scheduleLayouting();
	}

	public runTheTopTask(): boolean {
		const task = this._layoutTasks.shift();
		if (!task || !task.func) return false;

		app.enterRAF();

		try {
			task.func();
		} catch (ex) {
			console.error('LayoutingTask exception: ' + ex);
		}

		app.exitRAF();

		return true;
	}

	public cancelFrame() {
		if (this._requestedFrame) window.cancelAnimationFrame(this._requestedFrame);
		this._requestedFrame = null;
	}

	// Called by CanvasSectionContainer after it synchronously flushes tasks
	public triggerDrainCallbacks(): void {
		if (!this.hasTasksPending()) {
			this._runDrainCallbacks();
		}
	}

	// internal implementation below

	private _setupTimer() {
		this._layoutTaskFlush = setTimeout(
			this._flushLayoutingQueue.bind(this),
			this.MIN_TIMER_DELAY_MS,
		);
	}

	private _reachedTaskTimeout(start: number): boolean {
		const duration = performance.now() - start;
		if (duration > this.MAX_TASK_DURATION_MS) return true;
		return false;
	}

	private _flushLayoutingQueue(): void {
		this._layoutTaskFlush = null;
		if (!this.hasTasksPending()) {
			this._runDrainCallbacks();
			return;
		}

		this._requestedFrame = window.requestAnimationFrame(() => {
			this._requestedFrame = null;

			const start = performance.now();
			while (this.runTheTopTask()) {
				if (this._reachedTaskTimeout(start)) {
					this._scheduleLayouting();
					return;
				}
			}
			this._runDrainCallbacks();
		});
	}

	private _runDrainCallbacks(): void {
		app.enterRAF();

		const callbacks = this._drainCallbacks;
		this._drainCallbacks = [];
		for (const cb of callbacks) {
			try {
				cb();
			} catch (ex) {
				console.error('Drain callback exception: ' + ex);
			}
		}

		app.exitRAF();
	}

	private _scheduleLayouting(): void {
		if (this._layoutTaskFlush) return;
		this._setupTimer();
	}
}
