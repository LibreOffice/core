/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

type DefCallBack = () => void;
interface WSDServerInfo {
	Id: string;
	Version: string;
	Hash: string;
	Protocol: string;
	Options: string;
	TimeZone: string;
}

type MessageInterface =
	| string
	| ArrayBuffer
	| Blob
	| Uint8Array
	| Int8Array
	| DataView;

interface DelayedMessageInterface {
	msg: string;
}

interface CoolHTMLImageElement extends HTMLImageElement {
	completeTraceEvent?: CompleteTraceEvent;

	rawData?: Uint8Array;
	isKeyframe?: boolean;
}

interface SlurpMessageEvent extends MessageEvent {
	data: MessageInterface;
	imgBytes?: Uint8Array;
	imgIndex?: number;
	textMsg: string;
	isComplete(): boolean;
	image?: CoolHTMLImageElement;
	imageIsComplete?: boolean;
	callback?: DefCallBack;
	reason?: string;
}

interface MinimalMessageEvent {
	textMsg: string;
	callback?: JSDialogCallback | DefCallBack;
}
