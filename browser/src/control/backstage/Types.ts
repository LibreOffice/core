// @ts-strict-ignore -*- Mode: JavaScript; js-indent-level: 8; fill-column: 100 -*-

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
 * Shared types for the backstage view and its template files. With
 * tsconfig module: "none", these top-level interfaces are visible in any
 * .tsx file under browser/src/control/backstage/ without an import.
 */

interface BackstageTabConfig {
	id: string;
	label: string;
	type: 'view' | 'action' | 'separator';
	icon?: string;
	visible?: boolean;
	viewType?: 'home' | 'templates' | 'info' | 'export';
	actionType?:
		| 'open'
		| 'save'
		| 'saveas'
		| 'print'
		| 'share'
		| 'repair'
		| 'properties'
		| 'history'
		| 'options'
		| 'about';
}

interface TemplateTypeMap {
	writer: 'writer';
	calc: 'calc';
	impress: 'impress';
}

type TemplateType = TemplateTypeMap[keyof TemplateTypeMap];

interface TemplateData {
	id: string;
	name: string;
	type: TemplateType;
	path?: string;
	basename?: string;
	preview?: string;
	featured?: boolean;
	searchText: string;
}

interface TemplateManifestEntry {
	id?: string;
	name?: string;
	type?: string;
	category?: string;
	path: string;
	basename?: string | null;
	preview?: string | null;
	featured?: boolean;
}

interface TemplateManifest {
	templates?: TemplateManifestEntry[];
}

interface ExportFormatData {
	id: string;
	name: string;
	description: string;
}

interface ExportOptionItem {
	action: string;
	text: string;
	command?: string;
}

// todo: currently export as and downloadAs with pdf as not working, skipping for moment
interface ExportSections {
	exportAs: ExportOptionItem[];
	downloadAs: ExportOptionItem[];
}

/*
 * BackstageSVGIcons is a generated map of the form 'icon_name' -> 'SVG content'.
 * It is created as generated JS directly, and bundled, so that things work
 * together. We need to do it this way, because it is very complicated to
 * combine a compiled TS with generated TS in out-of-tree builds.
 */
declare const BackstageSVGIcons: Record<string, string>;
