/* -*- js-indent-level: 8 -*- */
/* global require Promise __dirname */

var fs = require('fs');
var path = require('path');
var list = require('./selectorList').list;

function copyFile(args) {
	return new Promise(function(resolve) {
		var sourceFile = args.sourceDir + args.fileName;
		var destFileName;
		if (args.destFileName) {
			destFileName = args.destFileName;
		} else {
			destFileName = args.fileName;
		}

		var destFile = args.destDir + destFileName;

		if (fs.existsSync(sourceFile)) {
			fs.mkdirSync(args.destDir, { recursive: true });
			if (fs.existsSync(destFile)) {
				fs.unlinkSync(destFile);
			}
			fs.writeFileSync(destFile, fs.readFileSync(sourceFile));
			resolve('File ${sourceFile} copied to ${destFile}');
		}
		resolve('File ${sourceFile} does not exist');
	});
}

function getSelectors(args) {
	if (args.mode === 'notebookbar') {
		return list[args.name][0];
	} else {
		return list[args.name][1];
	}
}

// Seed a debug user's browser settings on disk. The test WOPI server serves
// these back from per-user store test/data/presets/user/u-<userId>/ when the
// document is loaded with that &userid (see userPresetDir() in
// test/TestWopiFileServer.hpp). Writing the file matters beyond the initial
// value: it makes the client receive a browsersetting: message, which turns on
// useBrowserSetting so subsequent theme changes are persisted to the server -
// which is what the theme isolation/reload tests need.
function writeUserSetting(args) {
	return new Promise(function(resolve) {
		var dir = path.resolve(__dirname, '../../test/data/presets/user/u-' + String(args.userId));
		fs.mkdirSync(dir, { recursive: true });
		fs.writeFileSync(path.join(dir, 'browsersetting.json'), JSON.stringify(args.settings));
		resolve('wrote browsersetting.json for u-' + args.userId);
	});
}

module.exports.copyFile = copyFile;
module.exports.getSelectors = getSelectors;
module.exports.writeUserSetting = writeUserSetting;
