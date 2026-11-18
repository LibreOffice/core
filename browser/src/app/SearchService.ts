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

class SearchService {
	// TODO: avoid window.L.DomUtil.get(...)

	public searchNext() {
		this.search(window.L.DomUtil.get('search-input').value, false);
	}

	public searchPrevious() {
		this.search(window.L.DomUtil.get('search-input').value, true);
	}

	public search(
		text: string,
		backward?: boolean,
		replaceString?: string,
		command?: number,
		expand?: boolean,
	) {
		if (backward === undefined) {
			backward = false;
		}
		if (command === undefined) {
			command = 0;
		}
		if (replaceString === undefined) {
			replaceString = '';
		}
		if (
			window.L.Map.THIS._docLayer._searchResults &&
			text !== window.L.Map.THIS._docLayer._searchTerm
		) {
			window.L.Map.THIS._docLayer._clearSearchResults();
		}

		var searchCmd = {
			'SearchItem.SearchString': {
				type: 'string',
				value: undefined as any,
			},
			'SearchItem.ReplaceString': {
				type: 'string',
				value: undefined as any,
			},
			'SearchItem.Backward': {
				type: 'boolean',
				value: undefined as any,
			},
			'SearchItem.SearchStartPointX': {
				type: 'long',
				value: undefined as any,
			},
			'SearchItem.SearchStartPointY': {
				type: 'long',
				value: undefined as any,
			},
			'SearchItem.Command': {
				type: 'long',
				value: undefined as any,
			},
		};

		window.L.Map.THIS.fire('clearselection');

		Util.ensureValue(app.activeDocument);
		var searchStartPointX = app.activeDocument.activeLayout.viewedRectangle.x1;
		var searchStartPointY = app.activeDocument.activeLayout.viewedRectangle.y1;
		if (
			window.L.Map.THIS._docLayer &&
			window.L.Map.THIS._docLayer._lastSearchResult &&
			expand
		) {
			var strTwips =
				window.L.Map.THIS._docLayer._lastSearchResult.twipsRectangles.match(
					/\d+/g,
				);
			if (strTwips != null) {
				searchStartPointX = strTwips[0];
				searchStartPointY = strTwips[1];
			}
			this.resetSelection();
		}

		searchCmd['SearchItem.SearchString'].value = text;
		searchCmd['SearchItem.Backward'].value = backward;
		searchCmd['SearchItem.ReplaceString'].value = replaceString;
		searchCmd['SearchItem.SearchStartPointX'].value = searchStartPointX;
		searchCmd['SearchItem.SearchStartPointY'].value = searchStartPointY;
		searchCmd['SearchItem.Command'].value = command;
		app.socket.sendMessage(
			'uno .uno:ExecuteSearch ' + JSON.stringify(searchCmd),
		);
	}

	public highlightAll(text: string) {
		if (
			window.L.Map.THIS._docLayer._searchResults &&
			text === window.L.Map.THIS._docLayer._searchTerm
		) {
			return;
		}
		this.search(text, false, '1');
	}

	public resetSelection() {
		window.L.Map.THIS._docLayer._clearSearchResults();
		app.socket.sendMessage('resetselection');
	}
}

app.searchService = new SearchService();
