/* -*- tab-width: 4 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

enum EffectState {
	NotStarted,
	Playing,
	Ended,
}

class Effect {
	private nId: number;
	private eState: EffectState;

	constructor(nId: number = -1) {
		this.nId = nId;
		this.eState = EffectState.NotStarted;
	}

	getId() {
		return this.nId;
	}

	isMainEffect() {
		return this.nId === -1;
	}

	isPlaying() {
		return this.eState === EffectState.Playing;
	}

	isEnded() {
		return this.eState === EffectState.Ended;
	}

	start() {
		assert(
			this.eState === EffectState.NotStarted,
			'Effect.start: wrong state.',
		);
		this.eState = EffectState.Playing;
	}

	end() {
		assert(this.eState === EffectState.Playing, 'Effect.end: wrong state.');
		this.eState = EffectState.Ended;
	}
}
