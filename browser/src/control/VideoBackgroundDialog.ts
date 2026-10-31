/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
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

	// Each sample is videobackgrounds/<name>.webm, with its thumbnail at
	// images/videobackgrounds/<name>.png.
	private static getSamples(): Array<{ name: string; text: string }> {
		return [
			{ name: 'clouds-day', text: _('Clouds') },
			{ name: 'clouds-dusk', text: _('Clouds at dusk') },
			{ name: 'bokeh', text: _('Bokeh') },
		];
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
		const canRemove =
			app.map['stateChangeHandler'].getItemValue(
				'.uno:RemoveVideoBackground',
			) !== 'disabled';

		const json = uiManager._modalDialogJSON(id, _('Background Video'), true, [
			{
				id: id + '-info',
				type: 'fixedtext',
				text: _(
					'The video plays behind every slide that uses the master slide of this slide.',
				),
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
					selected: false,
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
		const fileName = name + '.webm';
		try {
			const response = await fetch(
				app.LOUtil.getURL('videobackgrounds/' + fileName),
			);
			if (!response.ok) throw new Error(response.statusText);
			const blob = await response.blob();
			app.map.fire('videobackground', {
				file: new File([blob], fileName, { type: 'video/webm' }),
			});
		} catch (error) {
			window.app.console.error(
				'VideoBackgroundDialog: cannot load ' + fileName + ': ' + error,
			);
		}
	}
}
