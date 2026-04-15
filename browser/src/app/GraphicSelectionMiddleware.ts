// @ts-strict-ignore
/* -*- js-indent-level: 8; fill-column: 100 -*- */

/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

declare var JSDialog: any;

class GraphicSelection {
	public static rectangle: cool.SimpleRectangle | null = null;
	public static extraInfo: any = null;
	public static selectionAngle: number = 0;
	public static handlesSection: ShapeHandlesSection = null;
	public static chartContextToolbarSelectStyle: ChartContextButtonSection =
		null;
	public static chartContextToolbarSaveStyle: ChartContextButtonSection = null;
	public static diagramButton: DiagramButtonSection = null;

	public static hasActiveSelection() {
		return this.rectangle !== null;
	}

	public static onUpdatePermission() {
		this.rectangle = null;
		this.updateGraphicSelection();
	}

	static resetSelectionRanges() {
		this.rectangle = null;
		this.extraInfo = null;

		if (this.handlesSection) {
			this.handlesSection.removeSubSections();
			app.sectionContainer.removeSection(this.handlesSection.name);
			this.handlesSection = null;
		}
	}

	// shows the video inside current selection marker
	static onEmbeddedVideoContent(textMsg: string) {
		if (!this.handlesSection) return;

		var videoDesc = JSON.parse(textMsg);

		if (this.hasActiveSelection()) {
			videoDesc.width = this.rectangle.cWidth;
			videoDesc.height = this.rectangle.cHeight;
		}

		// proxy cannot identify RouteToken if it is encoded
		var routeTokenIndex = videoDesc.url.indexOf('%26RouteToken=');
		if (routeTokenIndex != -1) {
			videoDesc.url = videoDesc.url.replace(
				'%26RouteToken=',
				'&amp;RouteToken=',
			);
		}

		var videoToInsert =
			'<?xml version="1.0" encoding="UTF-8"?>\
		<svg xmlns="http://www.w3.org/2000/svg" width="' +
			videoDesc.width +
			'" height="' +
			videoDesc.height +
			'">\
		<foreignObject overflow="visible" width="' +
			videoDesc.width +
			'" height="' +
			videoDesc.height +
			'">\
			<body xmlns="http://www.w3.org/1999/xhtml">\
				<video controls="controls" width="' +
			videoDesc.width +
			'" height="' +
			videoDesc.height +
			'">\
					<source src="' +
			videoDesc.url +
			'" type="' +
			videoDesc.mimeType +
			'"/>\
					<track src="' +
			videoDesc.srt +
			'" kind="subtitles"' +
			'"/>\
				</video>\
			</body>\
		</foreignObject>\
		</svg>';

		this.handlesSection.addEmbeddedVideo(videoToInsert);
	}

	static renderDarkOverlay() {
		var topLeft = new cool.Point(this.rectangle.pX1, this.rectangle.pY1);
		var bottomRight = new cool.Point(this.rectangle.pX2, this.rectangle.pY2);

		if (app.map._docLayer.isCalcRTL()) {
			// Dark overlays (like any other overlay) need regular document coordinates.
			// But in calc-rtl mode, charts (like shapes) have negative x document coordinate
			// internal representation.
			topLeft.x = Math.abs(topLeft.x);
			bottomRight.x = Math.abs(bottomRight.x);
		}

		var bounds = new cool.Bounds(topLeft, bottomRight);

		app.map._docLayer._oleCSelections.setPointSet(CPointSet.fromBounds(bounds));
	}

	// When a shape is selected, the rectangles of other shapes are also sent from the core side.
	// They are in twips units.
	static convertObjectRectangleTwipsToPixels() {
		if (this.extraInfo && this.extraInfo.ObjectRectangles) {
			for (let i = 0; i < this.extraInfo.ObjectRectangles.length; i++) {
				for (let j = 0; j < 4; j++)
					this.extraInfo.ObjectRectangles[i][j] *=
						app.twipsToPixels * app.impress.twipsCorrection;
			}
		}
	}

	static extractAndSetGraphicSelection(messageJSON: any) {
		var signX = app.map._docLayer.isCalcRTL() ? -1 : 1;
		var hasExtraInfo = messageJSON.length > 5;
		var hasGridOffset = false;
		var extraInfo = null;
		if (hasExtraInfo) {
			extraInfo = messageJSON[5];
			if (extraInfo.gridOffsetX || extraInfo.gridOffsetY) {
				app.map._docLayer._shapeGridOffset = new cool.SimplePoint(
					signX * extraInfo.gridOffsetX,
					extraInfo.gridOffsetY,
				);
				hasGridOffset = true;
			}
		}

		// Calc RTL: Negate positive X coordinates from core if grid offset is available.
		signX = hasGridOffset && app.map._docLayer.isCalcRTL() ? -1 : 1;
		this.rectangle = new cool.SimpleRectangle(
			signX * messageJSON[0],
			messageJSON[1],
			signX * messageJSON[2],
			messageJSON[3],
		);

		if (hasGridOffset)
			this.rectangle.moveBy([
				app.map._docLayer._shapeGridOffset.x,
				app.map._docLayer._shapeGridOffset.y,
			]);

		this.extraInfo = extraInfo;

		if (app.map._docLayer._docType === 'presentation')
			this.convertObjectRectangleTwipsToPixels();
	}

	/// Push down the graphic selection on non-first pages of scrolling PDF view.
	static transformGraphicSelection(messageJSON: any) {
		const docLayer = app.map._docLayer;
		const verticalOffset = docLayer.getFiledBasedViewVerticalOffset();
		if (!verticalOffset) {
			return;
		}

		// y
		messageJSON[1] += verticalOffset;

		const extraInfo = messageJSON[5];
		const rectangle = extraInfo?.handles?.kinds?.rectangle;
		if (!rectangle) {
			return;
		}

		for (const key of ['1', '2', '3', '4', '5', '6', '7', '8']) {
			const y = parseInt(rectangle[key][0].point.y);
			rectangle[key][0].point.y = y + verticalOffset;
		}
	}

	public static updateGraphicSelection() {
		if (this.hasActiveSelection()) {
			// Hide the keyboard on graphic selection, unless cursor is visible.
			// Don't interrupt editing in dialogs
			if (!JSDialog.IsAnyInputFocused())
				app.map.focus(app.file.textCursor.visible);

			let editMode = app.map.isEditMode();
			if (!editMode) {
				// If the just added signature line shape is selected, show the
				// graphic selection.
				editMode = this.extraInfo && this.extraInfo.isSignature;
			}
			if (!editMode) {
				return;
			}

			var extraInfo = this.extraInfo;
			let addHandlesSection = false;

			if (!this.handlesSection) addHandlesSection = true;
			else if (extraInfo.id !== this.handlesSection.sectionProperties.info.id) {
				// Another shape is selected.
				this.handlesSection.removeSubSections();
				app.sectionContainer.removeSection(this.handlesSection.name);
				this.handlesSection = null;
				addHandlesSection = true;
			}

			if (addHandlesSection) {
				this.handlesSection = new app.definitions.shapeHandlesSection({});
				app.sectionContainer.addSection(this.handlesSection);
			}

			this.handlesSection.setPosition(this.rectangle.pX1, this.rectangle.pY1);

			extraInfo.hasTableSelection =
				app.activeDocument.tableMiddleware.hasTableSelection(); // scaleSouthAndEastOnly

			this.handlesSection.refreshInfo(this.extraInfo);
			this.handlesSection.setShowSection(true);
			app.sectionContainer.requestReDraw();
		} else if (
			this.handlesSection &&
			app.sectionContainer.doesSectionExist(this.handlesSection.name)
		) {
			this.handlesSection.removeSubSections();
			app.sectionContainer.removeSection(this.handlesSection.name);
			this.handlesSection = null;
		}
		app.map._docLayer._updateCursorAndOverlay();
	}

	public static onShapeSelectionContent(textMsg: string) {
		textMsg = textMsg.substring('shapeselectioncontent:'.length + 1);

		var extraInfo = this.extraInfo;
		if (extraInfo && extraInfo.id) {
			app.map._cacheSVG[extraInfo.id] = textMsg;
		}

		// video is handled in _onEmbeddedVideoContent
		if (this.handlesSection) {
			if (this.handlesSection.sectionProperties.hasVideo)
				app.map._cacheSVG[extraInfo.id] = undefined;
			else this.handlesSection.setSVG(textMsg);

			if (!app.file.textCursor.visible) this.handlesSection.interactable = true;
		}
	}

	private static checkDiagramData() {
		if (GraphicSelection.extraInfo && GraphicSelection.extraInfo.isDiagram) {
			if (!GraphicSelection.diagramButton) {
				// need to create DiagramButtonSection
				const subSection = app.sectionContainer.getSectionWithName(
					this.handlesSection.sectionProperties.subSectionPrefix + 'diagram',
				);
				var halfWidth: number = 0;

				if (subSection && subSection.sectionProperties) {
					// get distance that frame occupies from frame object
					halfWidth = subSection.sectionProperties.ownInfo.halfWidth;
				}

				GraphicSelection.diagramButton = new DiagramButtonSection(
					halfWidth * app.twipsToPixels,
				);
				GraphicSelection.diagramButton.forceNextReposition();
				app.sectionContainer.addSection(GraphicSelection.diagramButton);
			}

			GraphicSelection.diagramButton.updatePosition();
		} else if (GraphicSelection.diagramButton) {
			app.sectionContainer.removeSection(GraphicSelection.diagramButton.name);
			GraphicSelection.diagramButton = null;
		}
	}

	private static checkChartData() {
		// Chart Context Buttons are disabled now.
		// we will probably enable it when chart styles will work fine.
		var disabled = true;
		if (disabled) return;
		if (
			GraphicSelection.extraInfo &&
			GraphicSelection.extraInfo.isChartPage &&
			GraphicSelection.extraInfo.isChartPage === true
		) {
			if (!GraphicSelection.chartContextToolbarSelectStyle) {
				GraphicSelection.chartContextToolbarSelectStyle =
					new ChartContextButtonSection(0);
				GraphicSelection.chartContextToolbarSaveStyle =
					new ChartContextButtonSection(1);
				GraphicSelection.chartContextToolbarSelectStyle.forceNextReposition();
				GraphicSelection.chartContextToolbarSaveStyle.forceNextReposition();
				app.sectionContainer.addSection(
					GraphicSelection.chartContextToolbarSelectStyle,
				);
				app.sectionContainer.addSection(
					GraphicSelection.chartContextToolbarSaveStyle,
				);
			}
			GraphicSelection.chartContextToolbarSelectStyle.updatePosition();
			GraphicSelection.chartContextToolbarSaveStyle.updatePosition();

			if (GraphicSelection.chartContextToolbarSelectStyle) {
				GraphicSelection.chartContextToolbarSelectStyle.showChartContextToolbar();
				GraphicSelection.chartContextToolbarSaveStyle.showChartContextToolbar();
			}
		} else if (GraphicSelection.chartContextToolbarSelectStyle) {
			app.sectionContainer.removeSection(
				GraphicSelection.chartContextToolbarSelectStyle.name,
			);
			app.sectionContainer.removeSection(
				GraphicSelection.chartContextToolbarSaveStyle.name,
			);
			GraphicSelection.chartContextToolbarSelectStyle = null;
			GraphicSelection.chartContextToolbarSaveStyle = null;
		}
	}

	public static onMessage(textMsg: string) {
		URLPopUpSection.closeURLPopUp();

		if (textMsg.match('EMPTY')) {
			this.resetSelectionRanges();
		} else if (textMsg.match('INPLACE EXIT')) {
			app.map._docLayer._oleCSelections.clear();
		} else if (textMsg.match('INPLACE')) {
			if (app.map._docLayer._oleCSelections.empty()) {
				textMsg = '[' + textMsg.substr('graphicselection:'.length) + ']';
				try {
					var msgData = JSON.parse(textMsg);
					if (msgData.length > 1) this.extractAndSetGraphicSelection(msgData);
				} catch (error) {
					window.app.console.warn('cannot parse graphicselection command');
				}
				this.renderDarkOverlay();

				this.rectangle = null;
				this.updateGraphicSelection();
			}
		} else {
			textMsg = '[' + textMsg.substr('graphicselection:'.length) + ']';
			msgData = JSON.parse(textMsg);

			this.transformGraphicSelection(msgData);

			this.extractAndSetGraphicSelection(msgData);

			// Update the dark overlay on zooming & scrolling
			if (!app.map._docLayer._oleCSelections.empty()) {
				app.map._docLayer._oleCSelections.clear();
				this.renderDarkOverlay();
			}

			this.selectionAngle = msgData.length > 4 ? msgData[4] : 0;

			if (this.extraInfo) {
				var dragInfo = this.extraInfo.dragInfo;
				if (dragInfo && dragInfo.dragMethod === 'PieSegmentDragging') {
					dragInfo.initialOffset /= 100.0;
					var dragDir = dragInfo.dragDirection;
					dragInfo.dragDirection = app.map._docLayer._twipsToPixels(
						new cool.Point(dragDir[0], dragDir[1]),
					);
					dragDir = dragInfo.dragDirection;
					dragInfo.range2 = dragDir.x * dragDir.x + dragDir.y * dragDir.y;
				}
			}

			// defaults
			var extraInfo = this.extraInfo;
			if (extraInfo) {
				if (extraInfo.isDraggable === undefined) extraInfo.isDraggable = true;
				if (extraInfo.isResizable === undefined) extraInfo.isResizable = true;
				if (extraInfo.isRotatable === undefined) extraInfo.isRotatable = true;
			}

			// Workaround for tdf#123874. For some reason the handling of the
			// shapeselectioncontent messages that we get back causes the WebKit process
			// to crash on iOS.

			// Note2: scroll to frame in writer would result an error:
			//   svgexport.cxx:810: ...UnknownPropertyException message: "Background
			var isFrame = extraInfo.type == 601 && !extraInfo.isWriterGraphic;

			if (
				!window.ThisIsTheiOSApp &&
				this.extraInfo.isDraggable &&
				!this.extraInfo.svg &&
				!isFrame
			) {
				app.socket.sendMessage('rendershapeselection mimetype=image/svg+xml');
			}

			// scroll to selected graphics, if it has no cursor
			if (
				!app.map._docLayer.isWriter() &&
				this.rectangle &&
				app.map._docLayer._allowViewJump()
			) {
				if (
					(!app.isPointVisibleInTheDisplayedArea([
						this.rectangle.x1,
						this.rectangle.y1,
					]) ||
						!app.isPointVisibleInTheDisplayedArea([
							this.rectangle.x2,
							this.rectangle.y2,
						])) &&
					!TextSelections.getEndRectangle() &&
					!(app.isFollowingEditor() || app.isFollowingUser()) &&
					!app.map.calcInputBarHasFocus()
				) {
					app.map._docLayer.scrollToPos(
						new cool.SimplePoint(this.rectangle.x1, this.rectangle.y1),
					);
				}
			}
		}

		// Graphics are by default complex selections, unless Core tells us otherwise.
		if (app.map._clip) app.map._clip.onComplexSelection('');

		// Reset text selection - important for textboxes in Impress
		if (app.map._docLayer._selectionContentRequest)
			clearTimeout(app.map._docLayer._selectionContentRequest);
		app.map._docLayer._onMessage('textselectioncontent:');

		this.updateGraphicSelection();

		if (msgData && msgData.length > 5) {
			var extraInfo = msgData[5];
			if (extraInfo.url !== undefined) {
				this.onEmbeddedVideoContent(JSON.stringify(extraInfo));
			}
		}

		GraphicSelection.checkChartData();
		GraphicSelection.checkDiagramData();
	}

	/*
		Some keywords: mousecontrol, _postMouseEvent, core side.
		Explanation:
			* User can edit a shape's text.
			* We need to send the mouse events to core in that case (for text selection, double click etc).
			* When cursor is visible, we assume text editing started.
			* And we set the "interactable" property of the shape handler section to false.
			* When "interactable" is false, canvas section container events pass through the section.
			* In our case, ShapeHandlesSection begins to be ignored.
			* And MouseControl catches all the events.
			* Users can select text, click, double click, triple click, quadruple click etc.
	*/
	public static onTextCursorVisibility(event: any) {
		if (this.hasActiveSelection()) {
			if (event.detail.visible) this.handlesSection.interactable = false;
			else this.handlesSection.interactable = true;
		}
	}
}

app.events.on(
	'updatepermission',
	GraphicSelection.onUpdatePermission.bind(GraphicSelection),
);

app.events.on(
	'TextCursorVisibility',
	GraphicSelection.onTextCursorVisibility.bind(GraphicSelection),
);

app.definitions.graphicSelection = GraphicSelection;
