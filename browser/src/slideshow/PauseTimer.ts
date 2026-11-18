// @ts-strict-ignore
/* -*- js-indent-level: 8 -*- */

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
 * PauseTimer helps display the countdown in pause mode, and we typically use it to repeat after x pauses
 */

declare var SlideShow: any;

abstract class PauseTimer {
	public abstract startTimer(): void;
	public abstract stopTimer(): void;
}

class PauseTimer2d implements PauseTimer {
	private onComplete: () => void;
	constructor(
		canvasContext: RenderContext2d,
		pauseDuration: number,
		onComplete: () => void,
	) {
		this.onComplete = onComplete;
	}

	public startTimer(): void {
		this.onComplete();
	}

	// eslint-disable-next-line @typescript-eslint/no-empty-function
	public stopTimer(): void {}
}

class PauseTimerGl extends StaticTextRenderer implements PauseTimer {
	private pauseTimeRemaining: number;
	private pauseDuration: number;
	private onComplete: () => void;
	private textCanvas: HTMLCanvasElement;
	private ctx: CanvasRenderingContext2D;

	constructor(
		canvasContext: RenderContextGl,
		pauseDuration: number,
		onComplete: () => void,
	) {
		super(canvasContext);
		this.pauseDuration = pauseDuration;
		this.pauseTimeRemaining = pauseDuration;
		this.onComplete = onComplete;

		if (this.context.isDisposed()) return;

		this.textCanvas = document.createElement('canvas');
		this.textCanvas.width = this.context.canvas.width;
		this.textCanvas.height = this.context.canvas.height;
		this.ctx = this.textCanvas.getContext('2d');

		this.textTexture = this.createTextTexture(this.getPauseTextContent());
		this.prepareTransition();
	}

	public startTimer(): void {
		if (this.context.isDisposed()) return;

		this.startTime = performance.now();
		requestAnimationFrame(this.animate.bind(this));
	}

	public stopTimer(): void {
		this.pauseTimeRemaining = 0;
		this.delete2dTextCanvas();
	}

	public animate(): void {
		if (this.context.isDisposed()) return;

		if (!this.textCanvas || !this.ctx) return;
		const currentTime = performance.now();
		const elapsedTime = (currentTime - this.startTime) / 1000;
		this.pauseTimeRemaining = Math.max(0, this.pauseDuration - elapsedTime);

		this.textTexture = this.createTextTexture(this.getPauseTextContent());

		this.render();
		requestAnimationFrame(this.animate.bind(this));

		if (this.pauseTimeRemaining <= 0) {
			this.onComplete();
			this.delete2dTextCanvas();
			return;
		}
	}

	public createTextTexture(displayText: string): WebGLTexture {
		if (this.context.isDisposed()) return null;

		this.clearCanvas();
		this.drawText(displayText);
		return this.load2dCanvasToGlCanvas(this.textCanvas);
	}

	private clearCanvas(): void {
		this.ctx.clearRect(0, 0, this.textCanvas.width, this.textCanvas.height);
		this.ctx.fillStyle = 'black';
		this.ctx.fillRect(0, 0, this.textCanvas.width, this.textCanvas.height);
	}

	// add text on off screen canvas...
	private drawText(displayText: string): void {
		if (this.context.isDisposed()) return;

		this.ctx.fillStyle = 'white';
		this.ctx.font = '20px sans-serif';
		this.ctx.textAlign = 'center';
		this.ctx.textBaseline = 'middle';
		this.ctx.fillText(
			displayText,
			this.textCanvas.width / 2,
			this.textCanvas.height / 2,
		);
	}

	public delete2dTextCanvas(): void {
		if (this.textCanvas) {
			this.textCanvas.remove();
			this.textCanvas = null;
			this.ctx = null;
		}
	}

	private getPauseTextContent(): string {
		return _('Pause... ( %SECONDS% )').replace(
			'%SECONDS%',
			Math.ceil(this.pauseTimeRemaining).toString(),
		);
	}
}

SlideShow.PauseTimer = PauseTimer;
SlideShow.PauseTimer2d = PauseTimer2d;
SlideShow.PauseTimerGl = PauseTimerGl;
