// @ts-strict-ignore
/* -*- js-indent-level: 8 -*- */
/* global app */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

interface DOMObjectLike {
	[index: string]: any;
	children?: DOMObjectLike[];
}

interface RectangleLike {
	top: number;
	left: number;
	bottom: number;
	right: number;
}

interface IconNameMap {
	[key: string]: string;
}

declare var DOMPurify: any;

// LOUtil contains various LO related utility functions used
// throughout the code.

class LOUtil {
	// Based on core.git's colordata.hxx: COL_AUTHOR1_DARK...COL_AUTHOR9_DARK
	// consisting of arrays of RGB values
	// Maybe move the color logic to separate file when it becomes complex
	public static darkColors = [
		[198, 146, 0],
		[6, 70, 162],
		[87, 157, 28],
		[105, 43, 157],
		[197, 0, 11],
		[0, 128, 128],
		[140, 132, 0],
		[53, 85, 107],
		[209, 118, 0],
	];

	// This list is based on supported filters defined in core in
	// filter/Configuration_filter.mk
	public static graphicMimeFilter = [
		// bitmaps (well-defined)
		'image/bmp',
		'image/gif',
		'image/jpeg',
		'image/png',
		'image/webp',
		'image/tiff',

		// vector graphics (canonical MIME)
		'image/svg+xml',
		'image/x-emf',
		'image/x-wmf',

		// PDF as image
		'application/pdf',

		// extensions for everything else (or as fallback)
		// file pickers don't recognize them by MIME type
		// Windows file picker doesn't recognize vector images by MIME type
		'.svg',
		'.svgz',
		'.emf',
		'.emz',
		'.wmf',
		'.wmz',
		'.eps',
		'.dxf',
		'.pct',
		'.pcx',
		'.pcd',
		'.psd',
		'.tga',
		'.ras',
		'.svm',
		'.met',
		'.pbm',
		'.pgm',
		'.ppm',
		'.xbm',
		'.xpm',
	];

	public static mediaMimeFilter = [
		'video/mp2t',
		'video/mp4',
		'video/mpeg',
		'video/ogg',
		'video/quicktime',
		'video/webm',
		'video/x-matroska',
		'video/x-ms-wmv',
		'video/x-msvideo',
		'audio/aac',
		'audio/flac',
		'audio/mp4',
		'audio/mpeg',
		'audio/ogg',
		'audio/wav',
	];

	// Not spreadsheet, presentation or drawing.
	public static documentMimeFilter = [
		'application/vnd.oasis.opendocument.text',
		'application/vnd.openxmlformats-officedocument.wordprocessingml.document',
		'application/msword',
		'text/rtf',
	];

	public static onRemoveHTMLElement(
		element: Element,
		onDetachCallback: () => void,
	) {
		const observer = new MutationObserver(function () {
			function isDetached(el: Element) {
				return !el.closest('html');
			}

			if (isDetached(element)) {
				onDetachCallback();
				observer.disconnect();
			}
		});

		observer.observe(document, {
			childList: true,
			subtree: true,
		});
	}

	public static startSpinner(
		spinnerCanvas: HTMLCanvasElement,
		spinnerSpeed: number,
	): NodeJS.Timer {
		spinnerCanvas.width = 50;
		spinnerCanvas.height = 50;

		const context = spinnerCanvas.getContext('2d');
		context.lineWidth = 8;
		context.strokeStyle = 'grey';
		const x = spinnerCanvas.width / 2;
		const y = spinnerCanvas.height / 2;
		const radius = y - context.lineWidth / 2;
		const spinnerInterval = setInterval(function () {
			context.clearRect(0, 0, x * 2, y * 2);
			// Move to center
			context.translate(x, y);
			context.rotate((spinnerSpeed * Math.PI) / 180);
			context.translate(-x, -y);
			context.beginPath();
			context.arc(x, y, radius, 0, Math.PI * 1.3);
			context.stroke();
		}, 30);

		return spinnerInterval;
	}

	public static getViewIdColor(viewId: number): number {
		const color = LOUtil.darkColors[(viewId + 1) % LOUtil.darkColors.length];
		return color[2] | (color[1] << 8) | (color[0] << 16);
	}

	public static rgbToHex(color: number) {
		return '#' + ('000000' + color.toString(16)).slice(-6);
	}

	public static stringToBounds(bounds: string): cool.Bounds {
		const numbers = bounds.match(/\d+/g);
		const topLeft = cool.Point.toPoint(
			parseInt(numbers[0]),
			parseInt(numbers[1]),
		);
		const bottomRight = topLeft.add(
			cool.Point.toPoint(parseInt(numbers[2]), parseInt(numbers[3])),
		);
		return cool.Bounds.toBounds(topLeft, bottomRight);
	}

	public static stringToRectangles(strRect: string): cool.Point[][] {
		const matches = strRect.match(/\d+/g);
		const rectangles: cool.Point[][] = [];
		if (matches !== null) {
			for (let itMatch = 0; itMatch < matches.length; itMatch += 4) {
				const topLeft = cool.Point.toPoint(
					parseInt(matches[itMatch]),
					parseInt(matches[itMatch + 1]),
				);
				const size = cool.Point.toPoint(
					parseInt(matches[itMatch + 2]),
					parseInt(matches[itMatch + 3]),
				);
				const topRight = topLeft.add(cool.Point.toPoint(size.x, 0));
				const bottomLeft = topLeft.add(cool.Point.toPoint(0, size.y));
				const bottomRight = topLeft.add(size);
				rectangles.push([bottomLeft, bottomRight, topLeft, topRight]);
			}
		}
		return rectangles;
	}

	// Map of locale → icon filenames that have locale-specific variants.
	private static localizedIcons: Record<string, string[]> = {
		ar: ['lc_chapternumberingdialog.svg', 'lc_linenumberingdialog.svg'],
		de: [
			'lc_bold.svg',
			'lc_italic.svg',
			'lc_numberformatdecdecimals.svg',
			'lc_numberformatdecimal.svg',
			'lc_numberformatincdecimals.svg',
			'lc_numberformatthousands.svg',
		],
		es: ['lc_bold.svg', 'lc_underline.svg', 'lc_underlinedouble.svg'],
		fr: ['lc_bold.svg'],
		hu: ['lc_italic.svg', 'lc_underline.svg', 'lc_underlinedouble.svg'],
		it: ['lc_italic.svg'],
		km: [
			'lc_bold.svg',
			'lc_italic.svg',
			'lc_underline.svg',
			'lc_underlinedouble.svg',
		],
		ko: [
			'lc_bold.svg',
			'lc_charfontname.svg',
			'lc_color.svg',
			'lc_datasort.svg',
			'lc_editstyle.svg',
			'lc_fontdialog.svg',
			'lc_grow.svg',
			'lc_italic.svg',
			'lc_overline.svg',
			'lc_shadowed.svg',
			'lc_shrink.svg',
			'lc_sortascending.svg',
			'lc_sortdescending.svg',
			'lc_strikeout.svg',
			'lc_stylenewbyexample.svg',
			'lc_styleupdatebyexample.svg',
			'lc_text.svg',
			'lc_underline.svg',
			'lc_underlinedouble.svg',
			'lc_verticaltext.svg',
		],
		nl: ['lc_bold.svg', 'lc_underline.svg', 'lc_underlinedouble.svg'],
		pl: ['lc_underline.svg', 'lc_underlinedouble.svg'],
		ru: ['lc_bold.svg', 'lc_underline.svg', 'lc_underlinedouble.svg'],
		sl: ['lc_bold.svg', 'lc_italic.svg'],
		tr: ['lc_italic.svg'],
	};

	private static getUILanguageCode(): string {
		const lang = (String as any).locale || '';
		return lang.split('-')[0].split('_')[0].toLowerCase();
	}

	// Some items will only be present in dark mode so we will not check errors
	// for those in other mode.
	public static onlydarkModeItems: string[] = ['invertbackground'];

	// Common images used in all modes, so the default one will be used.
	public static commonItems: string[] = [
		'serverauditok',
		'serverauditerror',
		'compact_customanimation',
		'slideshow-exit',
		'slideshow-slideNext',
		'slideshow-slidePrevious',
	];

	// Helper function to strip '.svg' suffix and 'lc_' prefix.
	public static stripName(name: string): string {
		// Remove the '.svg' suffix.
		var strippedName = name.replace(/\.svg$/, '');

		// Remove the 'lc_' prefix if it exists.
		if (strippedName.startsWith('lc_')) {
			strippedName = strippedName.substring(3);
		}

		return strippedName;
	}

	public static isDarkModeItem(name: string): boolean {
		const strippedName = LOUtil.stripName(name);

		// Check if the stripped name is in the onlydarkModeItems array.
		return LOUtil.onlydarkModeItems.includes(strippedName);
	}

	public static isCommonForAllMode(name: string): boolean {
		const strippedName = LOUtil.stripName(name);

		// Check if the stripped name is in the commonItems array.
		return LOUtil.commonItems.includes(strippedName);
	}

	/// unwind things to get a good absolute URL.
	public static getURL(path: string): string {
		if (path === '') return '';
		const customWindow = window as any;
		if (customWindow.host === '' && customWindow.serviceRoot === '') {
			// Mobile / desktop app: return a relative path so it resolves
			// against the page's file:// origin rather than the filesystem root.
			if (path.startsWith('/')) return path.substring(1);
			return path;
		}

		let url = customWindow.makeHttpUrl('/browser/' + customWindow.versionPath);
		if (path.substr(0, 1) !== '/') url += '/';

		url += path;
		return url;
	}

	public static setImage(
		img: HTMLImageElement,
		name: string,
		map: any,
		imageIsLayoutCritical?: boolean,
	): void {
		const setupIcon = function () {
			img.src = LOUtil.getImageURL(name);
			LOUtil.checkIfImageExists(img, imageIsLayoutCritical);
		};
		setupIcon();

		map.on('themechanged', setupIcon);
	}

	public static setUserImage(
		img: HTMLImageElement,
		map: any,
		viewId: number,
	): void {
		// set avatar image if it exist in user extract info.
		const defaultImage = LOUtil.getImageURL('user.svg');
		const viewInfo = map._viewInfo[viewId];
		if (
			viewInfo !== undefined &&
			viewInfo.userextrainfo !== undefined &&
			viewInfo.userextrainfo.avatar !== undefined
		) {
			// set user avatar.
			img.src = viewInfo.userextrainfo.avatar;
			// Track if error event is already bound to this image.
			img.addEventListener(
				'error',
				function () {
					img.src = defaultImage;
					LOUtil.checkIfImageExists(img, true);
				},
				{ once: true },
			);
			return;
		}
		img.src = defaultImage;
		LOUtil.checkIfImageExists(img, true);
	}

	public static getImageURL(imgName: string) {
		let defaultImageURL = LOUtil.getURL('images/' + imgName);

		// Check if the image name is in the commonItems list and return the normal image path
		if (LOUtil.isCommonForAllMode(imgName)) {
			return defaultImageURL;
		}

		const lang = LOUtil.getUILanguageCode();
		const hasLocalized = lang && LOUtil.localizedIcons[lang]?.includes(imgName);

		if ((window as any).prefs.getBoolean('darkTheme')) {
			if (hasLocalized)
				return LOUtil.getURL('images/dark/' + lang + '/' + imgName);
			return LOUtil.getURL('images/dark/' + imgName);
		}

		if (hasLocalized) return LOUtil.getURL('images/' + lang + '/' + imgName);

		const dummyEmptyImg =
			'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNkYAAAAAYAAjCB0C8AAAAASUVORK5CYII=';
		defaultImageURL = LOUtil.isDarkModeItem(imgName)
			? dummyEmptyImg
			: defaultImageURL;
		return defaultImageURL;
	}

	public static getIconNameOfCommand(name: string, noCommad?: boolean) {
		if (!name) return '';

		var alreadyClean = noCommad;
		var cleanName = name;

		if (!alreadyClean || alreadyClean !== true) {
			var prefixLength = '.uno:'.length;
			if (name.substr(0, prefixLength) == '.uno:')
				cleanName = name.substr(prefixLength);
			cleanName = encodeURIComponent(cleanName).replace(/%/g, '');
			cleanName = cleanName.toLowerCase();
		}

		// Skip icon lookup for numeric-only IDs (JSDialog artifacts like 1, 5, 65535)
		// and core sr* resource IDs (like sr20006)
		if (/^\d+$/.test(cleanName) || /^sr\d+$/.test(cleanName)) return '';

		// Skip icon lookup for overflow button pseudo-commands
		if (cleanName.startsWith('overflow-button-')) return '';

		// Strip 'sc_' prefix from sidebar controller command names
		// (core's small-command icon prefix that doesn't apply to COOL)
		if (cleanName.startsWith('sc_')) cleanName = cleanName.substring(3);

		var iconURLAliases: IconNameMap = {
			// lc_closemobile.svg is generated when loading in NB mode then
			// switch to compact mode: 1st hidden element in the top toolbar
			closemobile: 'closedocmobile',
			'file-saveas': 'saveas',
			savegraphic: 'saveas',
			saveimagetowopi: 'saveasremote',
			'home-search': 'recsearch',
			searchdialog3finitialfocusreplace3abool3dtrue: 'searchreplace',
			'addmb-menu': 'ok',
			closetablet: 'view',
			defineprintarea: 'menuprintranges',
			deleteprintarea: 'delete',
			sheetrighttoleft: 'pararighttoleft',
			alignleft: 'leftpara',
			alignright: 'rightpara',
			alignhorizontalcenter: 'centerpara',
			alignblock: 'justifypara',
			formatsparklinemenu: 'insertsparkline',
			insertdatecontentcontrol: 'datefield',
			editheaderandfooter: 'headerandfooter',
			insertfooter: 'insertpagefooter',
			exportas: 'saveas',
			insertheaderfooter: 'headerandfooter',
			previoustrackedchange: 'prevrecord',
			fieldtransparency: 'linetransparency',
			lb_glow_transparency: 'linetransparency',
			settransparency: 'linetransparency',
			field_transparency: 'linetransparency',
			selectionlanugagedefault: 'updateall',
			connectortoolbox: 'connectorlines',
			conditionalformatdialog: 'conditionalformatmenu',
			groupoutlinemenu: 'group',
			paperwidth: 'pagewidth',
			charspacing: 'spacing',
			fontworkcharacterspacingfloater: 'spacing',
			tablesort: 'datasort',
			spellcheckignoreall: 'spelling',
			deleterowbreak: 'delbreakmenu',
			alignmentpropertypanel: 'alignvcenter',
			cellvertcenter: 'alignvcenter',
			charbackcolor: 'backcolor',
			charmapcontrol: 'insertsymbol',
			insertrowsafter: 'insertrowsmenu',
			insertobjectchart: 'drawchart',
			textpropertypanel: 'sidebartextpanel',
			textbodyparastyle: 'parastyle',
			spacepara15: 'linespacing',
			orientationdegrees: 'rotation',
			clearoutline: 'delete',
			docsign: 'editdoc',
			editmenu: 'editdoc',
			drawtext: 'text',
			inserttextbox: 'text',
			accepttrackedchanges: 'acceptchanges',
			accepttrackedchange: 'acceptchanges',
			chartlinepanel: 'linestyle',
			linepropertypanel: 'linestyle',
			xlinestyle: 'linestyle',
			listspropertypanel: 'outlinebullet',
			shadowpropertypanel: 'shadowed',
			incrementlevel: 'outlineleft',
			menurowheight: 'rowheight',
			menumargins: 'pagemargin',
			menuorientation: 'orientation',
			menupagesizescalc: 'pagesize',
			menupagesizeswriter: 'pagesize',
			setoptimalrowheight: 'rowheight',
			cellverttop: 'aligntop',
			scalignmentpropertypanel: 'aligntop',
			hyperlinkdialog: 'inserthyperlink',
			remotelink: 'inserthyperlink',
			remoteaicontent: 'sdrespageobjs',
			openhyperlinkoncursor: 'inserthyperlink',
			pageformatdialog: 'pagedialog',
			backgroundcolor: 'fillcolor',
			cellappearancepropertypanel: 'fillcolor',
			formatarea: 'fillcolor',
			glowcolor: 'fillcolor',
			sccellappearancepropertypanel: 'fillcolor',
			insertcolumnsafter: 'insertcolumnsmenu',
			insertnonbreakingspace: 'formattingmark',
			insertcurrentdate: 'datefield',
			insertdatefieldfix: 'datefield',
			insertdatefield: 'datefield',
			insertdatefieldvar: 'datefield',
			setparagraphlanguagemenu: 'spelldialog',
			spellingandgrammardialog: 'spelldialog',
			styleapply3fstyle3astring3ddefault26familyname3astring3dcellstyles:
				'fontcolor',
			fontworkgalleryfloater: 'fontworkpropertypanel',
			insertfieldctrl: 'insertfield',
			pagenumberwizard: 'insertpagenumberfield',
			entirerow: 'fromrow',
			insertcheckboxcontentcontrol: 'checkbox',
			cellvertbottom: 'alignbottom',
			insertcurrenttime: 'inserttimefield',
			inserttimefieldfix: 'inserttimefield',
			inserttimefieldvar: 'inserttimefield',
			cancelformula: 'cancel',
			resetattributes: 'setdefault',
			tabledialog: 'tablemenu',
			insertindexesentry: 'insertmultiindex',
			paperheight: 'pageheight',
			masterslidespanel: 'masterslide',
			slidemasterpage: 'masterslide',
			tabledeletemenu: 'deletetable',
			insertcalctable: 'inserttable',
			removecalctable: 'deletetable',
			calculatedfieldrun: 'functiondialog',
			databasesettings: 'tabledesign',
			tracechangemode: 'trackchanges',
			deleteallannotation: 'deleteallnotes',
			sdtabledesignpanel: 'tabledesign',
			tableeditpanel: 'tabledesign',
			tableautofitmenu: 'columnwidth',
			menucolumnwidth: 'columnwidth',
			hyphenation: 'hyphenate',
			validatedialogsa11y: 'validation',
			validatesidebara11y: 'validation',
			objectbackone: 'behindobject',
			deleteannotation: 'deletenote',
			areapropertypanel: 'chartareapanel',
			'downloadas-png': 'insertgraphic',
			decrementlevel: 'outlineright',
			acceptformula: 'ok',
			insertannotation: 'shownote',
			insertcomment: 'shownote',
			objecttitledescription: 'shownote',
			namegroup: 'shownote',
			incrementindent: 'leftindent',
			outlineup: 'moveup',
			charttypepanel: 'diagramtype',
			arrangeframemenu: 'arrangemenu',
			bringtofront: 'arrangemenu',
			scnumberformatpropertypanel: 'numberformatincdecimals',
			graphicpropertypanel: 'graphicdialog',
			rotateflipmenu: 'rotateleft',
			outlinedown: 'movedown',
			nexttrackedchange: 'nextrecord',
			toggleorientation: 'orientation',
			configuredialog: 'sidebar',
			modifypage: 'sidebar',
			parapropertypanel: 'paragraphdialog',
			tablecellbackgroundcolor: 'fillcolor',
			zoteroArtwork: 'zoteroThesis',
			zoteroAudioRecording: 'zoteroThesis',
			zoteroBill: 'zoteroThesis',
			zoteroBlogPost: 'zoteroThesis',
			zoteroBookSection: 'zoteroBook',
			zoteroCase: 'zoteroThesis',
			zoteroConferencePaper: 'zoteroThesis',
			zoteroDictionaryEntry: 'zoteroThesis',
			zoteroDocument: 'zoteroThesis',
			zoteroEmail: 'zoteroThesis',
			zoteroEncyclopediaArticle: 'zoteroThesis',
			zoteroFilm: 'zoteroThesis',
			zoteroForumPost: 'zoteroThesis',
			zoteroHearing: 'zoteroThesis',
			zoteroInstantMessage: 'zoteroThesis',
			zoteroInterview: 'zoteroThesis',
			zoteroLetter: 'zoteroThesis',
			zoteroMagazineArticle: 'zoteroThesis',
			zoteroWebpage: 'zoteroThesis',
			zoteroaddeditcitation: 'insertauthoritiesentry',
			zoteroaddnote: 'addcitationnote',
			zoterorefresh: 'updateall',
			zoterounlink: 'unlinkcitation',
			zoteroaddeditbibliography: 'addeditbibliography',
			zoteroeditbibliography: 'addeditbibliography',
			zoterosetdocprefs: 'formproperties',
			'sidebardeck.propertydeck': 'sidebar',
			// Fix issue #6145 by adding aliases for the PDF and EPUB icons
			// The fix for issues #6103 and #6104 changes the name of these
			// icons so map the new names to the old names.
			'downloadas-pdf': 'exportpdf',
			'downloadas-direct-pdf': 'exportdirectpdf',
			'downloadas-epub': 'exportepub',
			languagestatusmenu: 'languagemenu',
			cancelsearch: 'cancel',
			printoptions: 'print',
			togglesheetgrid: 'show',
			toggleprintgrid: 'printgrid',
			exportdirectpdf: 'exportpdf',
			textcolumnspropertypanel: 'entirecolumn',
			'sidebardeck.stylelistdeck': 'editstyle',
			assignlayout3fwhatlayout3along3d20: 'layout00',
			assignlayout3fwhatlayout3along3d0: 'layout01',
			assignlayout3fwhatlayout3along3d1: 'layout02',
			assignlayout3fwhatlayout3along3d3: 'layout03',
			assignlayout3fwhatlayout3along3d19: 'layout04',
			assignlayout3fwhatlayout3along3d32: 'layout05',
			assignlayout3fwhatlayout3along3d15: 'layout06',
			assignlayout3fwhatlayout3along3d12: 'layout07',
			assignlayout3fwhatlayout3along3d16: 'layout08',
			assignlayout3fwhatlayout3along3d14: 'layout09',
			assignlayout3fwhatlayout3along3d18: 'layout10',
			assignlayout3fwhatlayout3along3d34: 'layout11',
			assignlayout3fwhatlayout3along3d27: 'layout12',
			assignlayout3fwhatlayout3along3d28: 'layout13',
			assignlayout3fwhatlayout3along3d29: 'layout02',
			assignlayout3fwhatlayout3along3d30: 'layout03',
			insertpage3fwhatlayout3along3d20: 'layout00',
			insertpage3fwhatlayout3along3d0: 'layout01',
			insertpage3fwhatlayout3along3d1: 'layout02',
			insertpage3fwhatlayout3along3d3: 'layout03',
			insertpage3fwhatlayout3along3d19: 'layout04',
			insertpage3fwhatlayout3along3d32: 'layout05',
			insertpage3fwhatlayout3along3d15: 'layout06',
			insertpage3fwhatlayout3along3d12: 'layout07',
			insertpage3fwhatlayout3along3d16: 'layout08',
			insertpage3fwhatlayout3along3d14: 'layout09',
			insertpage3fwhatlayout3along3d18: 'layout10',
			insertpage3fwhatlayout3along3d34: 'layout11',
			insertpage3fwhatlayout3along3d27: 'layout12',
			insertpage3fwhatlayout3along3d28: 'layout13',
			insertpage3fwhatlayout3along3d29: 'layout02',
			insertpage3fwhatlayout3along3d30: 'layout03',
			graphicfilterinvert: 'graphicfilterinvert',
			graphicfilterpopart: 'graphicfilterpopart',
			graphicfilterremovenoise: 'graphicfilterremovenoise',
			graphicfiltersharpen: 'graphicfiltersharpen',
			graphicfiltersobel: 'graphicfiltersobel',
			effects: 'pictureeffectsmenu',
			showmultiplepages: 'multipageview',
			showtwopages: 'multipageview',
			fitwidthzoom: 'pagewidth',
			open: 'formularesfapopen',
			'exportas-pdf': 'exportpdf',
			'exportas-epub': 'exportepub',
			'fullscreen-drawing': 'presentation',
			endnotedialog: 'footnotedialog',
			updateallindexes: 'insertmultiindex',
			formatframemenu: 'framedialog',
		};
		if (iconURLAliases[cleanName]) {
			cleanName = iconURLAliases[cleanName];
		}

		return 'lc_' + cleanName + '.svg';
	}

	public static checkIfImageExists(
		imageElement: HTMLImageElement,
		imageIsLayoutCritical?: boolean,
	): void {
		imageElement.addEventListener('error', function (e: any) {
			if (e.loUtilProcessed) {
				return;
			}

			if (
				imageElement.src &&
				imageElement.src.includes('/images/branding/dark/')
			) {
				imageElement.src = imageElement.src.replace(
					'/images/branding/dark/',
					'/images/dark/',
				);
				e.loUtilProcessed = true;
				return;
			}
			if (
				imageElement.src &&
				(imageElement.src.includes('/images/dark/') ||
					imageElement.src.includes('/images/branding/'))
			) {
				imageElement.src = imageElement.src.replace(
					'/images/dark/',
					'/images/',
				);
				imageElement.src = imageElement.src.replace(
					'/images/branding/',
					'/images/',
				);
				e.loUtilProcessed = true;
				return;
			}

			if (imageIsLayoutCritical) {
				imageElement.src =
					'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNkYAAAAAYAAjCB0C8AAAAASUVORK5CYII=';
				// We cannot set visibility: hidden because that would hide
				// other attributes of the image, e.g. its border.
				e.loUtilProcessed = true;
				return;
			}

			imageElement.style.display = 'none';
			e.loUtilProcessed = true;
		});
	}

	/// oldFileName = Example.odt, suffix = new
	/// returns: Example_new.odt
	public static generateNewFileName(
		oldFileName: string,
		suffix: string,
	): string {
		const idx = oldFileName.lastIndexOf('.');
		return oldFileName.substring(0, idx) + suffix + oldFileName.substring(idx);
	}

	public static commandWithoutIcon: string[] = [
		'InsertPageHeader',
		'InsertPageFooter',
		'FLD_COL_NUMBER',
		'MTR_FLD_COL_SPACING',
		'rows',
		'cols',
		'None',
	];

	public static existsIconForCommand(
		command: string,
		docType: string,
	): boolean {
		const commandName = command.startsWith('.uno:')
			? command.substring('.uno:'.length)
			: command;
		const res = !LOUtil.commandWithoutIcon.find(function (el: string) {
			return el.startsWith(commandName);
		});
		if (commandName.indexOf('?') !== -1) {
			if (
				commandName.indexOf('SpellCheckIgnore') !== -1 ||
				commandName.indexOf('SpellCheckIgnoreAll') !== -1
			)
				return true;

			if (
				(docType === 'spreadsheet' || docType === 'presentation') &&
				commandName.indexOf('LanguageStatus') !== -1
			)
				return true;

			if (
				commandName ===
					'LanguageStatus?Language:string=Current_LANGUAGE_NONE' ||
				commandName ===
					'LanguageStatus?Language:string=Current_RESET_LANGUAGES' ||
				commandName ===
					'LanguageStatus?Language:string=Paragraph_LANGUAGE_NONE' ||
				commandName ===
					'LanguageStatus?Language:string=Paragraph_RESET_LANGUAGES'
			)
				return true;

			return false;
		}
		return res;
	}

	/// Searching in JSON trees for data with a given field.
	public static findItemWithAttributeRecursive(
		node: DOMObjectLike,
		idName: string,
		idValue: any,
	): DOMObjectLike | null {
		let found: DOMObjectLike | null = null;
		if (node[idName] === idValue) return node;
		if (node.children) {
			for (var i = 0; !found && i < node.children.length; i++)
				found = LOUtil.findItemWithAttributeRecursive(
					node.children[i],
					idName,
					idValue,
				);
		}
		return found;
	}

	/// Searching in JSON trees for an identifier and return the index in parent.
	public static findIndexInParentByAttribute(
		node: DOMObjectLike,
		idName: string,
		idValue: any,
	): number {
		if (node.children) {
			for (var i = 0; i < node.children.length; i++)
				if (node.children[i][idName] === idValue) return i;
		}
		return -1;
	}

	public static _doRectanglesIntersect(
		rectangle1: number[],
		rectangle2: number[],
	): boolean {
		// Format: (x, y, w, h).
		// Don't use equality in comparison, that's not an intersection.
		if (
			Math.abs(
				rectangle1[0] +
					rectangle1[2] * 0.5 -
					(rectangle2[0] + rectangle2[2] * 0.5),
			) <
			0.5 * (rectangle1[2] + rectangle2[2])
		) {
			if (
				Math.abs(
					rectangle1[1] +
						rectangle1[3] * 0.5 -
						(rectangle2[1] + rectangle2[3] * 0.5),
				) <
				0.5 * (rectangle1[3] + rectangle2[3])
			)
				return true;
			else return false;
		} else return false;
	}

	// Returns the intersecting area of 2 rectangles. Rectangle format: (x, y, w, h). Return format is the same or null.
	public static _getIntersectionRectangle(
		rectangle1: number[],
		rectangle2: number[],
	): number[] | null {
		if (this._doRectanglesIntersect(rectangle1, rectangle2)) {
			var x = rectangle1[0] > rectangle2[0] ? rectangle1[0] : rectangle2[0];
			var y = rectangle1[1] > rectangle2[1] ? rectangle1[1] : rectangle2[1];
			var w =
				rectangle1[0] + rectangle1[2] < rectangle2[0] + rectangle2[2]
					? rectangle1[0] + rectangle1[2] - x
					: rectangle2[0] + rectangle2[2] - x;
			var h =
				rectangle1[1] + rectangle1[3] < rectangle2[1] + rectangle2[3]
					? rectangle1[1] + rectangle1[3] - y
					: rectangle2[1] + rectangle2[3] - y;

			return [x, y, w, h];
		} else return null;
	}

	public static getFileExtension(map: any): string {
		const filename: string = map['wopi'].BaseFileName;
		return filename.substring(filename.lastIndexOf('.') + 1);
	}

	public static isFileODF(map: any): boolean {
		var ext = LOUtil.getFileExtension(map);
		return (
			ext === 'odt' ||
			ext === 'ods' ||
			ext === 'odp' ||
			ext === 'odg' ||
			ext === 'fodt' ||
			ext === 'fods' ||
			ext === 'fodp' ||
			ext === 'fodg'
		);
	}

	public static containsDOMRect(
		viewRect: RectangleLike,
		rect: RectangleLike,
	): boolean {
		return (
			rect.top >= viewRect.top &&
			rect.right <= viewRect.right &&
			rect.bottom <= viewRect.bottom &&
			rect.left >= viewRect.left
		);
	}

	public static Rectangle = cool.Rectangle;
	public static createRectangle = cool.createRectangle;

	public static sanitize(
		html: string,
		profile: 'html' | 'svg' = 'html',
	): string {
		if (DOMPurify.isSupported) {
			if (profile === 'svg') {
				return DOMPurify.sanitize(html, {
					// Enable both SVG and HTML profiles to support HTML content inside foreignObject
					USE_PROFILES: { svg: true, svgFilters: true, html: true },
					ADD_TAGS: ['foreignObject'],
					// Allow ODF namespaced attributes used by LibreOffice SVG export.
					// See: core/filter/source/svg/svgexport.cxx for the full list.
					ADD_ATTR: [
						'ooo:background-visibility',
						'ooo:date-time-field',
						'ooo:date-time-format',
						'ooo:date-time-visibility',
						'ooo:display-name',
						'ooo:footer-field',
						'ooo:footer-visibility',
						'ooo:has-custom-background',
						'ooo:has-transition',
						'ooo:header-field',
						'ooo:id-list',
						'ooo:master',
						'ooo:master-objects-visibility',
						'ooo:name',
						'ooo:number-of-slides',
						'ooo:numbering-type',
						'ooo:page-number-visibility',
						'ooo:page-numbering-type',
						'ooo:slide',
						'ooo:slide-duration',
						'ooo:start-slide-number',
						'ooo:text-adjust',
						'ooo:use-positioned-chars',
					],
					// Allow HTML content inside foreignObject (for embedded video)
					// See: https://github.com/cure53/DOMPurify/issues/1002
					HTML_INTEGRATION_POINTS: { foreignobject: true },
				});
			}
			return DOMPurify.sanitize(html, { USE_PROFILES: { [profile]: true } });
		}
		return '';
	}

	public static getDocumentLogoClass(docType: string) {
		let iconClass: string;
		let iconTooltip: string;
		if (docType === 'text') {
			iconClass = 'writer-icon-img';
			iconTooltip = 'Writer';
		} else if (docType === 'spreadsheet') {
			iconClass = 'calc-icon-img';
			iconTooltip = 'Calc';
		} else if (docType === 'presentation') {
			iconClass = 'impress-icon-img';
			iconTooltip = 'Impress';
		} else if (docType === 'drawing') {
			iconClass = 'draw-icon-img';
			iconTooltip = 'Draw';
		}

		return [iconClass, iconTooltip];
	}
}

app.LOUtil = LOUtil;
