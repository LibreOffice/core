/*
 * Copyright (c) 2012 Jason Polites
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.polites.android;

public class Animator extends Thread {

	private GestureImageView view;
	private Animation animation;
	private boolean running = false;
	private boolean active = false;
	private long lastTime = -1L;

	public Animator(GestureImageView view, String threadName) {
		super(threadName);
		this.view = view;
	}

	@Override
	public void run() {

		running = true;

		while(running) {

			while(active && animation != null) {
				long time = System.currentTimeMillis();
				active = animation.update(view, time - lastTime);
				view.redraw();
				lastTime = time;

				while(active) {
					try {
						if(view.waitForDraw(32)) { // 30Htz
							break;
						}
					}
					catch (InterruptedException ignore) {
						active = false;
					}
				}
			}

			synchronized(this) {
				if(running) {
					try {
						wait();
					}
					catch (InterruptedException ignore) {}
				}
			}
		}
	}

	public synchronized void finish() {
		running = false;
		active = false;
		notifyAll();
	}

	public void play(Animation transformer) {
		if(active) {
			cancel();
		}
		this.animation = transformer;

		activate();
	}

	public synchronized void activate() {
		lastTime = System.currentTimeMillis();
		active = true;
		notifyAll();
	}

	public void cancel() {
		active = false;
	}
}
