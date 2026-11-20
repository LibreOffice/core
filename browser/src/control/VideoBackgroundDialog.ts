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
 * VideoBackgroundDialog offers our sample videos, a video of the user's own, and removal, for the
 * background video of the master slide of the current slide.
 */

class VideoBackgroundDialog {
	private static readonly id = 'VideoBackgroundDialog';

	// Each sample is videobackgrounds/<name>.mp4, with its thumbnail at
	// images/videobackgrounds/<name>.png.
	private static getSamples(): Array<{ name: string; text: string }> {
		return [
			{ name: 'clouds-day', text: _('Clouds') },
			{ name: 'clouds-dusk', text: _('Clouds at dusk') },
			{ name: 'bokeh', text: _('Bokeh') },
		];
	}

	// The index of the sample that fileName, the file name of a background video in the document,
	// was made from, or -1. A sample goes in under its own file name. A server puts 16
	// hexadecimal digits and a hyphen before that name, and a name the document holds already
	// gets a number before the extension.
	private static sampleIndexOf(
		samples: Array<{ name: string; text: string }>,
		fileName: string,
	): number {
		return samples.findIndex((sample) =>
			new RegExp('^([0-9A-Fa-f]{16}-)?' + sample.name + '[0-9]*\\.mp4$').test(
				fileName,
			),
		);
	}

	public static show(): void {
		const uiManager = app.map.uiManager;
		const id = VideoBackgroundDialog.id;
		const dialogId = uiManager.generateModalId(id);
		const samplesId = id + '-samples';
		const ownId = id + '-own';
		const removeId = id + '-remove';
		const cancelId = id + '-cancel';
		const samples = VideoBackgroundDialog.getSamples();
		// The state is the file name of the background video, or "disabled" when there is none.
		const videoState = app.map['stateChangeHandler'].getItemValue(
			'.uno:RemoveVideoBackground',
		);
		const canRemove = videoState !== 'disabled';
		const selectedIndex =
			canRemove && typeof videoState === 'string'
				? VideoBackgroundDialog.sampleIndexOf(samples, videoState)
				: -1;

		const json = uiManager._modalDialogJSON(id, _('Background Video'), true, [
			{
				id: id + '-info',
				type: 'fixedtext',
				text: _('All slides that use this master slide get this video.'),
			},
			{
				id: samplesId,
				type: 'iconview',
				singleclickactivate: true,
				textWithIconEnabled: true,
				entries: samples.map((sample, index) => ({
					row: index,
					text: sample.text,
					tooltip: sample.text,
					image: app.LOUtil.getURL(
						'images/videobackgrounds/' + sample.name + '.png',
					),
					selected: index === selectedIndex,
				})),
			},
			{
				id: '',
				type: 'buttonbox',
				text: '',
				enabled: true,
				vertical: false,
				layoutstyle: 'end',
				children: [
					{
						id: removeId,
						type: 'pushbutton',
						text: _('Remove Background Video'),
						hidden: !canRemove,
					},
					{
						id: ownId,
						type: 'pushbutton',
						text: _('Choose Your Own Video...'),
					},
					{ id: cancelId, type: 'pushbutton', text: _('Cancel') },
				],
			},
		]);

		uiManager.showModal(
			json,
			[
				{
					id: samplesId,
					type: 'activate',
					func: (
						objectType: string,
						eventType: string,
						object: any,
						row: number,
					) => {
						uiManager.closeModal(dialogId);
						VideoBackgroundDialog.insertSample(samples[row].name);
					},
				},
				{
					id: ownId,
					func: () => {
						uiManager.closeModal(dialogId);
						app.LOUtil.openFilePicker('videobackground');
					},
				},
				{
					id: removeId,
					func: () => {
						uiManager.closeModal(dialogId);
						app.map.sendUnoCommand('.uno:RemoveVideoBackground');
					},
				},
			],
			cancelId,
		);
	}

	// The sample goes in the same way as a video the user picks: the file is sent to the
	// document.
	private static async insertSample(name: string): Promise<void> {
		const fileName = name + '.mp4';
		try {
			const response = await fetch(
				app.LOUtil.getURL('videobackgrounds/' + fileName),
			);
			// WebKit answers a file: URL it has read with status 0.
			const isRead =
				response.ok ||
				(response.status === 0 && response.url.startsWith('file:'));
			if (!isRead) throw new Error(response.statusText);
			const blob = await response.blob();
			app.map.fire('videobackground', {
				file: new File([blob], fileName, { type: 'video/mp4' }),
			});
		} catch (error) {
			window.app.console.error(
				'VideoBackgroundDialog: cannot load ' + fileName + ': ' + error,
			);
		}
	}
}
