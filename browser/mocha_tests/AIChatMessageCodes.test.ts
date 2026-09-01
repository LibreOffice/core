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

// The AI chat sidebar localizes server-sent messages by the stable code each
// frame carries (errorCode / statusKey / displayCode); the English text on
// the wire is only a fallback for transient version skew. A code emitted by
// wsd/AIChatSession.cpp without a matching entry in the sidebar's translation
// maps would reach users untranslated, so this test parses both sources and
// fails on any code missing from the client, and on any stale client entry
// the server no longer sends.
describe('AI chat message codes', function () {
	const fs = require('fs');
	const path = require('path');

	const serverSource: string = fs.readFileSync(
		path.join(__dirname, '..', '..', '..', 'wsd', 'AIChatSession.cpp'),
		'utf8',
	);
	const clientSource: string = fs.readFileSync(
		path.join(__dirname, '..', '..', 'src', 'control', 'Control.AIChatSidebar.ts'),
		'utf8',
	);

	function addMatches(codes: Set<string>, source: string, re: RegExp): void {
		let m: RegExpExecArray | null;
		while ((m = re.exec(source)) !== null) codes.add(m[1]);
	}

	// The argument list of every call of the named function, skipping the
	// function's own definition. Relies on the calls containing no ';'.
	function callArgs(source: string, fnName: string): string[] {
		const out: string[] = [];
		const re = new RegExp(fnName + '\\(([^;]*?)\\);', 'g');
		let m: RegExpExecArray | null;
		while ((m = re.exec(source)) !== null) {
			if (source[m.index - 1] === ':') continue; // AIChatSession::fnName(...)
			out.push(m[1]);
		}
		return out;
	}

	// Bare camelCase string literals in the given argument lists. Message
	// codes are the only such literals: the English fallback text beside them
	// always contains a space or punctuation.
	function bareCodeLiterals(argLists: string[]): Set<string> {
		const codes = new Set<string>();
		for (const args of argLists) addMatches(codes, args, /"([a-z][a-zA-Z]*)"/g);
		return codes;
	}

	// The body of a method of Control.AIChatSidebar.ts, located by its
	// declaration text and the file's two-tab method indentation.
	function clientMethodBody(marker: string): string {
		const from = clientSource.indexOf(marker);
		nodeassert.ok(from >= 0, `not found in the client source: ${marker}`);
		const to = clientSource.indexOf('\n\t\t}', from);
		nodeassert.ok(to > from, `method close not found after: ${marker}`);
		return clientSource.slice(from, to);
	}

	function clientMapKeys(marker: string): Set<string> {
		const keys = new Set<string>();
		addMatches(keys, clientMethodBody(marker), /^\s*(\w+): \(\) =>/gm);
		return keys;
	}

	function missingFrom(codes: Set<string>, map: Set<string>): string[] {
		return Array.from(codes)
			.filter((code) => !map.has(code))
			.sort();
	}

	function serverErrorCodes(): Set<string> {
		const codes = new Set<string>();
		// Literal first argument of sendChatError().
		addMatches(codes, serverSource, /sendChatError\(\s*"(\w+)"/g);
		// ImageGenRequest / ImageGenResult errorCode assignments, forwarded to
		// sendChatError() and sendImageResult() as variables.
		addMatches(codes, serverSource, /\.errorCode = "(\w+)"/g);
		// The ChatError struct literals mapHttpStatusToError() returns.
		const from = serverSource.indexOf('AIChatSession::mapHttpStatusToError');
		nodeassert.ok(from >= 0, 'mapHttpStatusToError not found in the server source');
		const to = serverSource.indexOf('\n}', from);
		addMatches(codes, serverSource.slice(from, to), /\{ "(\w+)"/g);
		return codes;
	}

	const errorCodes = serverErrorCodes();
	const progressKeys = bareCodeLiterals(callArgs(serverSource, 'sendToolProgress'));
	const displayCodes = bareCodeLiterals(callArgs(serverSource, 'sendChatResult'));

	const errorMap = clientMapKeys('private translateChatError(');
	const progressMap = clientMapKeys('private translateProgress(');
	const displayCases = new Set<string>();
	addMatches(displayCases, clientMethodBody('private translateDisplay('), /case '(\w+)':/g);

	// Guard the extraction itself: if a refactor breaks one of the source
	// patterns above, the sets shrink and the sync checks turn vacuous, so
	// pin one known code per pattern and a floor on the total.
	it('extracts the message codes from both sources', function () {
		nodeassert.ok(errorCodes.has('aiNotConfigured'), 'sendChatError literals not found');
		nodeassert.ok(errorCodes.has('imageSettingsNotConfigured'), 'errorCode assignments not found');
		nodeassert.ok(errorCodes.has('apiInvalidKey'), 'mapHttpStatusToError codes not found');
		nodeassert.ok(errorCodes.size >= 30, `implausibly few error codes: ${errorCodes.size}`);
		nodeassert.ok(progressKeys.has('thinking'), 'sendToolProgress keys not found');
		nodeassert.ok(displayCodes.has('deckReady'), 'sendChatResult display codes not found');
	});

	it('translates every server error code', function () {
		nodeassert.deepEqual(
			missingFrom(errorCodes, errorMap),
			[],
			'sendChatError codes missing from translateChatError in Control.AIChatSidebar.ts',
		);
	});

	it('translates every server progress key', function () {
		nodeassert.deepEqual(
			missingFrom(progressKeys, progressMap),
			[],
			'sendToolProgress keys missing from translateProgress in Control.AIChatSidebar.ts',
		);
	});

	it('translates every server display code', function () {
		nodeassert.deepEqual(
			missingFrom(displayCodes, displayCases),
			[],
			'sendChatResult display codes missing from translateDisplay in Control.AIChatSidebar.ts',
		);
	});

	it('carries no stale client entries', function () {
		nodeassert.deepEqual(
			missingFrom(errorMap, errorCodes),
			[],
			'translateChatError entries the server never sends - remove them or emit them',
		);
		nodeassert.deepEqual(
			missingFrom(progressMap, progressKeys),
			[],
			'translateProgress entries the server never sends - remove them or emit them',
		);
		nodeassert.deepEqual(
			missingFrom(displayCases, displayCodes),
			[],
			'translateDisplay cases the server never sends - remove them or emit them',
		);
	});
});
