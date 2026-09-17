/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
var vm = require("vm");
var fs = require("fs");
var tmp = require('tmp');
const WebSocket = require('ws');

if (process.argv.length < 3 ||
    process.argv[2] == '--help') {
	console.debug('load.js <ssl_true_or_false> <abs_op_builddir> <abs-path-to-file> [bookmark] [port]');
	process.exit(0);
}

var ssl_flag = process.argv[2];
var top_builddir = process.argv[3];

var to_load;
if (process.argv.length > 4) {
	to_load = process.argv[4]
} else {
	var content = fs.readFileSync(top_builddir + '/test/data/perf-test-edit.odt');
	var tmpObj = tmp.fileSync({ mode: 0644, prefix: 'perf-test-', postfix: '.odt' });
	to_load = tmpObj.name;
	fs.writeFileSync(to_load, content);
}

var bookmark;
if (process.argv.length > 5) {
	bookmark = process.argv[5];
}

let port = 9980;
if (process.argv.length > 6) {
	port = process.argv[6];
}

let typing_speed = 30;
if (process.argv.length > 7) {
	typing_speed = parseInt(process.argv[7]);
}

let typing_duration = 5000;
if (process.argv.length > 8) {
	typing_duration = parseInt(process.argv[8]);
}

let record_stats = false;
if (process.argv.length > 9) {
	record_stats = process.argv[9] === 'true';
}

let single_view = false;
if (process.argv.length > 10) {
	single_view = process.argv[10] === 'true';
}
/*
global.console = {
  log: () => {},
  error: () => {},
  warn: () => {},
  info: () => {},
  debug: () => {},
  log2: () => {}
};
*/
// jsdom for browser emulation
const jsdom = require('jsdom');
const { JSDOM } = jsdom;

var data = fs.readFileSync(top_builddir + '/browser/dist/cool.html', {encoding: 'utf8'});

data = data.replace(/%UI_THEME%/g, 'light');

if (ssl_flag === 'true')
    data = data.replace(/%HOST%/g, `wss://localhost:${port}`);
else
    data = data.replace(/%HOST%/g, `ws://localhost:${port}`);
data = data.replace(/%SERVICE_ROOT%\/browser\/%VERSION%/g, top_builddir + '/browser/dist');
data = data.replace(/%SERVICE_ROOT%/g, '');
data = data.replace(/%HEXIFY_URL%/g, '""');
data = data.replace(/%VERSION%/g, 'dist');
data = data.replace(/%ACCESS_TOKEN%/g, '');
data = data.replace(/%ACCESS_TOKEN_TTL%/g, '0');
data = data.replace(/%ACCESS_HEADER%/g, '');
data = data.replace(/%POSTMESSAGE_ORIGIN%/g, '');
data = data.replace(/%RELAY_ORIGIN%/g, '');
data = data.replace(/%BROWSER_LOGGING%/g, 'true');
data = data.replace(/%COOLWSD_VERSION%/g, 'loadjs');
data = data.replace(/%ENABLE_WELCOME_MSG%/g, 'false');
data = data.replace(/%AUTO_SHOW_WELCOME%/g, 'false');
data = data.replace(/%AUTO_SHOW_FEEDBACK%/g, 'false');
data = data.replace(/%ENABLE_UPDATE_NOTIFICATION%/g, 'false');
data = data.replace(/%USER_INTERFACE_MODE%/g, '');
data = data.replace(/%USE_INTEGRATION_THEME%/g, 'true');
data = data.replace(/%STATUSBAR_SAVE_INDICATOR%/g, 'false');
data = data.replace(/%ENABLE_MACROS_EXECUTION%/g, '');
data = data.replace(/%OUT_OF_FOCUS_TIMEOUT_SECS%/g, '1000000');
data = data.replace(/%IDLE_TIMEOUT_SECS%/g, '1000000');
data = data.replace(/%MIN_SAVED_MESSAGE_TIMEOUT_SECS%/g, '6');
data = data.replace(/%PROTOCOL_DEBUG%/g, 'true');
data = data.replace(/%FRAME_ANCESTORS%/g, '');
data = data.replace(/%SOCKET_PROXY%/g, 'false');
data = data.replace(/%UI_DEFAULTS%/g, '{}');
data = data.replace(/%CHECK_FILE_INFO_OVERRIDE%/g, 'false');
data = data.replace(/%DEEPL_ENABLED%/g, 'false');
data = data.replace(/%ZOTERO_ENABLED%/g, 'false');
data = data.replace(/%DOCUMENT_SIGNING_ENABLED%/g, 'false');
data = data.replace(/%SAVED_UI_STATE%/g, 'false');
data = data.replace(/%WASM_ENABLED%/g, 'false');
data = data.replace(/%INDIRECTION_URL%/g, '');
data = data.replace(/%GEOLOCATION_SETUP%/g, 'false');
data = data.replace(/%CANVAS_SLIDESHOW_ENABLED%/g, 'true');

data = data.replace(/%WELCOME_URL%/g, '');
data = data.replace(/%FEEDBACK_URL%/g, '');
data = data.replace(/%BUYPRODUCT_URL%/g, '');
// data = data.replace(/<!--%CSS_VARIABLES%-->/g, '');
data = data.replace(/<!--%CSS_VARIABLES%-->/g, 'OnJvb3Qgey0tY28tcHJpbWFyeS10ZXh0OiNmZmZmZmY7LS1jby1wcmltYXJ5LWVsZW1lbnQ6IzVjMjk4MzstLWNvLXRleHQtYWNjZW50OiM1YzI5ODM7LS1jby1wcmltYXJ5LWxpZ2h0OiNlZWU5ZjI7LS1jby1wcmltYXJ5LWVsZW1lbnQtbGlnaHQ6I2VlZTlmMjstLWNvLWNvbG9yLWVycm9yOiNEQjA2MDY7LS1jby1jb2xvci13YXJuaW5nOiNBMzcyMDA7LS1jby1jb2xvci1zdWNjZXNzOiMyZDdiNDE7LS1jby1ib3JkZXItcmFkaXVzOjRweDstLWNvLWJvcmRlci1yYWRpdXMtbGFyZ2U6OHB4Oy0tY28tbG9hZGluZy1saWdodDojY2NjY2NjOy0tY28tbG9hZGluZy1kYXJrOiM0NDQ0NDQ7LS1jby1ib3gtc2hhZG93OnJnYmEoNzcsNzcsNzcsIDAuNSk7LS1jby1ib3JkZXI6I2VkZWRlZDstLWNvLWJvcmRlci1kYXJrOiNkYmRiZGI7LS1jby1ib3JkZXItcmFkaXVzLXBpbGw6MTAwcHg7LS1uYy1saWdodC1jb2xvci1tYWluLWJhY2tncm91bmQ6I2ZmZmZmZjstLW5jLWRhcmstY29sb3ItbWFpbi1iYWNrZ3JvdW5kOiMxNzE3MTc7LS1uYy1saWdodC1jb2xvci1tYWluLWJhY2tncm91bmQtcmdiOjI1NSwyNTUsMjU1Oy0tbmMtZGFyay1jb2xvci1tYWluLWJhY2tncm91bmQtcmdiOjIzLDIzLDIzOy0tbmMtbGlnaHQtY29sb3ItbWFpbi1iYWNrZ3JvdW5kLXRyYW5zbHVjZW50OnJnYmEoMjU1LDI1NSwyNTUsIC45Nyk7LS1uYy1kYXJrLWNvbG9yLW1haW4tYmFja2dyb3VuZC10cmFuc2x1Y2VudDpyZ2JhKDIzLDIzLDIzLCAuOTcpOy0tbmMtbGlnaHQtY29sb3ItbWFpbi1iYWNrZ3JvdW5kLWJsdXI6cmdiYSgyNTUsMjU1LDI1NSwgLjgpOy0tbmMtZGFyay1jb2xvci1tYWluLWJhY2tncm91bmQtYmx1cjpyZ2JhKDIzLDIzLDIzLCAuODUpOy0tbmMtbGlnaHQtY29sb3ItbWFpbi10ZXh0OiMyMjIyMjI7LS1uYy1kYXJrLWNvbG9yLW1haW4tdGV4dDojRUJFQkVCOy0tbmMtbGlnaHQtY29sb3ItdGV4dC1tYXhjb250cmFzdDojNmI2YjZiOy0tbmMtZGFyay1jb2xvci10ZXh0LW1heGNvbnRyYXN0OiM5OTk5OTk7LS1uYy1saWdodC1jb2xvci1ib3gtc2hhZG93OnJnYmEoNzcsNzcsNzcsIDAuNSk7LS1uYy1kYXJrLWNvbG9yLWJveC1zaGFkb3c6IzAwMDAwMDstLW5jLWxpZ2h0LWNvbG9yLWJveC1zaGFkb3ctcmdiOjc3LDc3LDc3Oy0tbmMtZGFyay1jb2xvci1ib3gtc2hhZG93LXJnYjowLDAsMDstLW5jLWxpZ2h0LWRlZmF1bHQtZm9udC1zaXplOjE1cHg7LS1uYy1kYXJrLWRlZmF1bHQtZm9udC1zaXplOjE1cHg7LS1uYy1saWdodC1ib3JkZXItcmFkaXVzOjRweDstLW5jLWRhcmstYm9yZGVyLXJhZGl1czo0cHg7LS1uYy1saWdodC1ib3JkZXItcmFkaXVzLWxhcmdlOjhweDstLW5jLWRhcmstYm9yZGVyLXJhZGl1cy1sYXJnZTo4cHg7LS1uYy1saWdodC1ib3JkZXItcmFkaXVzLXJvdW5kZWQ6MjhweDstLW5jLWRhcmstYm9yZGVyLXJhZGl1cy1yb3VuZGVkOjI4cHg7LS1uYy1saWdodC1ib3JkZXItcmFkaXVzLXBpbGw6MTAwcHg7LS1uYy1kYXJrLWJvcmRlci1yYWRpdXMtcGlsbDoxMDBweDstLW5jLWxpZ2h0LWRlZmF1bHQtY2xpY2thYmxlLWFyZWE6MzRweDstLW5jLWRhcmstZGVmYXVsdC1jbGlja2FibGUtYXJlYTozNHB4Oy0tbmMtbGlnaHQtZGVmYXVsdC1saW5lLWhlaWdodDoxLjU7LS1uYy1kYXJrLWRlZmF1bHQtbGluZS1oZWlnaHQ6MS41Oy0tbmMtbGlnaHQtZGVmYXVsdC1ncmlkLWJhc2VsaW5lOjRweDstLW5jLWRhcmstZGVmYXVsdC1ncmlkLWJhc2VsaW5lOjRweDstLW5jLWxpZ2h0LWNvbG9yLXByaW1hcnk6IzVjMjk4MzstLW5jLWRhcmstY29sb3ItcHJpbWFyeTojNWMyOTgzOy0tbmMtbGlnaHQtY29sb3ItcHJpbWFyeS10ZXh0OiNmZmZmZmY7LS1uYy1kYXJrLWNvbG9yLXByaW1hcnktdGV4dDojZmZmZmZmOy0tbmMtbGlnaHQtY29sb3ItcHJpbWFyeS1ob3ZlcjojN2M1MzliOy0tbmMtZGFyay1jb2xvci1wcmltYXJ5LWhvdmVyOiM0ZTI1NmQ7LS1uYy1saWdodC1jb2xvci1wcmltYXJ5LWxpZ2h0OiNlZWU5ZjI7LS1uYy1kYXJrLWNvbG9yLXByaW1hcnktbGlnaHQ6IzI1MjAyYTstLW5jLWxpZ2h0LWNvbG9yLXByaW1hcnktbGlnaHQtdGV4dDojMjQxMDM0Oy0tbmMtZGFyay1jb2xvci1wcmltYXJ5LWxpZ2h0LXRleHQ6I2JkYTljZDstLW5jLWxpZ2h0LWNvbG9yLXByaW1hcnktbGlnaHQtaG92ZXI6I2UzZGZlNzstLW5jLWRhcmstY29sb3ItcHJpbWFyeS1saWdodC1ob3ZlcjojMmUyYTMzOy0tbmMtbGlnaHQtY29sb3ItcHJpbWFyeS1lbGVtZW50OiM1YzI5ODM7LS1uYy1kYXJrLWNvbG9yLXByaW1hcnktZWxlbWVudDojYWM3NmQ2Oy0tbmMtbGlnaHQtY29sb3ItcHJpbWFyeS1lbGVtZW50LWhvdmVyOiM1MTI0NzM7LS1uYy1kYXJrLWNvbG9yLXByaW1hcnktZWxlbWVudC1ob3ZlcjojYjY4NmRiOy0tbmMtbGlnaHQtY29sb3ItcHJpbWFyeS1lbGVtZW50LXRleHQ6I2ZmZmZmZjstLW5jLWRhcmstY29sb3ItcHJpbWFyeS1lbGVtZW50LXRleHQ6IzAwMDAwMDstLW5jLWxpZ2h0LWNvbG9yLXByaW1hcnktZWxlbWVudC1saWdodDojZWVlOWYyOy0tbmMtZGFyay1jb2xvci1wcmltYXJ5LWVsZW1lbnQtbGlnaHQ6IzI1MjAyYTstLW5jLWxpZ2h0LWNvbG9yLXByaW1hcnktZWxlbWVudC1saWdodC1ob3ZlcjojZTNkZmU3Oy0tbmMtZGFyay1jb2xvci1wcmltYXJ5LWVsZW1lbnQtbGlnaHQtaG92ZXI6IzJlMmEzMzstLW5jLWxpZ2h0LWNvbG9yLXByaW1hcnktZWxlbWVudC1saWdodC10ZXh0OiMyNDEwMzQ7LS1uYy1kYXJrLWNvbG9yLXByaW1hcnktZWxlbWVudC1saWdodC10ZXh0OiNkZGM4ZWU7LS1uYy1saWdodC1jb2xvci1wcmltYXJ5LWVsZW1lbnQtdGV4dC1kYXJrOiNmNWY1ZjU7LS1uYy1kYXJrLWNvbG9yLXByaW1hcnktZWxlbWVudC10ZXh0LWRhcms6IzBhMGEwYTstLW5jLWxpZ2h0LXByaW1hcnktaW52ZXJ0LWlmLWJyaWdodDpubzstLW5jLWRhcmstcHJpbWFyeS1pbnZlcnQtaWYtYnJpZ2h0OmludmVydCgxMDAlKTstLW5jLWxpZ2h0LXByaW1hcnktaW52ZXJ0LWlmLWRhcms6aW52ZXJ0KDEpOy0tbmMtZGFyay1wcmltYXJ5LWludmVydC1pZi1kYXJrOmludmVydCgxKTstLW5jLWxpZ2h0LWJhY2tncm91bmQtaW52ZXJ0LWlmLWJyaWdodDppbnZlcnQoMTAwJSk7LS1uYy1kYXJrLWJhY2tncm91bmQtaW52ZXJ0LWlmLWJyaWdodDpubzstLW5jLWxpZ2h0LWJhY2tncm91bmQtaW52ZXJ0LWlmLWRhcms6bm87LS1uYy1kYXJrLWJhY2tncm91bmQtaW52ZXJ0LWlmLWRhcms6aW52ZXJ0KDEwMCUpOy0tbmMtY3VzdG9tLWxvZ286aHR0cHMlM0ElMkYlMkZzaGFyZS5jb2xsYWJvcmEuY29tJTJGaW5kZXgucGhwJTJGYXBwcyUyRnRoZW1pbmclMkZpbWFnZSUyRmxvZ29oZWFkZXIlM0Z2JTNEMjY7LS1uYy1sb2dvLWJhY2tncm91bmQ6dHJhbnNwYXJlbnQ7LS1uYy1sb2dvLWRpc3BsYXk6YmxvY2s7fQ==">');

data = data.replace(/%PRODUCT_BRANDING_NAME%/g, '');
data = data.replace(/%PRODUCT_BRANDING_URL%/g, '');

data = data.replace(/%BRANDING_THEME%/g, 'cool_brand');

window = new JSDOM(data, {
				runScripts: 'dangerously',
				verbose: false,
				pretendToBeVisual: false,
				includeNodeLocations: false,
				url: 'file:///tmp/notthere/cool.html?file_path=file:///' + to_load,
				resources: 'usable',
				beforeParse(window) {
					console.debug('Before script parsing');
				},
				done(errors, window) {
					console.debug('Errors ' + errors);
				}
		   }).window;

// Make it possible to mock sizing properties
Object.defineProperty(window.HTMLElement.prototype, "clientWidth", {
	get: function() {
		return this.___clientWidth || 0;
	}
});
Object.defineProperty(window.HTMLElement.prototype, "clientHeight", {
	get: function() {
		return this.___clientHeight || 0;
	}
});

process.stderr.write('Finished bootstrapping');
console.debug('Window size ' + window.innerWidth + 'x' + window.innerHeight);

window.HTMLElement.prototype.getBoundingClientRect = function() {
//	console.debug('getBoundingClientRect for ' + this.id);
	return {
		width: 0, height: 0, top: 0, left: 0
	};
};

// nodejs requires rejectUnauthorized to be set to cope with our https
window.createWebSocket = function(uri) {
        if ('processCoolUrl' in window) {
                uri = window.processCoolUrl({ url: uri, type: 'ws' });
        }

        if (global.socketProxy) {
                return new global.ProxySocket(uri);
        } else {
		// FIXME: rejectUnauthorized: false for SSL?
                return new WebSocket(uri);
        }
};

function sleep(ms)
{
	return new Promise(r => setTimeout(r, ms));
}
var docLoaded = false;
processedTiles = [];
let socketMessageCount = 0;

function dumpStats() {
	let len = processedTiles.length;
	let skipFirst = Math.floor(len * 0.1);
	let delays = [];
	const bucketCount = 10;
	let buckets = new Array(bucketCount).fill(0);
	const bucketSize = 20;
	for (let i = skipFirst; i < len; ++i) {
		let delay = processedTiles[i] - processedTiles[i-1];
		delays.push(delay);
		buckets[Math.min(Math.trunc(delay / bucketSize), bucketCount-1)]++;
	}
	let output = '';
	let now = new Date();
	output += now.getTime() + '\n';
	output += now.toUTCString() + '\n';
	let delayList = `Delay list=${delays.join()}`;
	output += delayList;
	output += '\n';
	for (var i = 0; i < bucketCount; i++) {
		if (i == bucketCount - 1)
			output += `>${i * 20}ms     \t=${buckets[i]} elements\n`;
		else
			output += `${i * 20}-${(i+1)*20}ms     \t=${buckets[i]} elements\n`;
	}
	output += '\n';
	output += `Bucket count=${buckets.length}\n`;
	output += `Bucket size=${bucketSize}ms\n`;
	output += `Num of samples=${delays.length}\n`;
	output += `Delay between each event=${typing_speed}ms\n`;
	output += `Duration=${typing_duration}ms\n`;
	output += `Total num of incoming socket messages=${socketMessageCount}\n`;
	output += `Single view=${single_view}\n`;
	fs.writeFile(top_builddir + '/browser/test/tilestats.txt', output, function (err) {
		if (err) console.log('tilestats: error dumping stats to file!', err);
		else console.log('tilestats: finished dumping the stats to file!');
	});
	process.stderr.write(output);
}

window.onload = function() {
	console.debug('socket ' + window.app.socket);
	var map = window.app.map;

	console.debug('Initialize / size map pieces ' + map);

	// Force some sizes onto key pieces:
	map.innerWidth = 1920;
	map.innerHeight = 1080
	map._container.___clientWidth = 1920;
	map._container.___clientHeight = 1080;

	map.on('docloaded', function(){
		if (docLoaded) return;
		docLoaded = true;
		console.debug('document loaded' +
			      ' mobile=' + window.L.Browser.mobile +
			      ' desktop=' + window.mode.isDesktop() + ' now running\n');
		setTimeout(async function() {
			if (bookmark)
			{
				let nodeIndex = parseInt(bookmark.substring(bookmark.lastIndexOf('_')+1));
				nodeIndex = nodeIndex - 1; // starts from 1
				console.debug('Jump to bookmark ' + bookmark);
				var cmd = {
					'Bookmark': { 'type': 'string', 'value': bookmark }
				};
				window.app.socket.sendMessage('uno .uno:JumpToMark ' + JSON.stringify(cmd));
				// set a mixed zoom levels: %50, %100, %200
				// tile sizes:
				let tileSizes = [7963, 3840, 1852];
				let zoomLevels = [50, 100, 200];
				let index = nodeIndex % tileSizes.length;
				// send zoom request
				console.debug('sending zoom request: %', zoomLevels[index]);
				let zoomMessage = `clientzoom tilepixelwidth=256 tilepixelheight=256 tiletwipwidth=${tileSizes[index]} tiletwipheight=${tileSizes[index]}`;
				console.debug(zoomMessage);
				window.app.socket.sendMessage(zoomMessage);
				await sleep(500);
				// mash the keyboard:
				let dummyInput = 'askdjf ,asdhflkas r;we f;akdn.adh ;o wh;fa he;qw e.fkahsd ;vbawe.kguday;f vas.,mdb kaery kejraerga';
				dummyInput = dummyInput.repeat(10);
				map.focus();
				let inputIndex = 0;
				if (record_stats) {
					console.log('tilestats: recording tile delays starting in one second..');
					await sleep(1000);
					console.log('tilestats: recording started..');
				}
				let timeStart = new Date().getTime();
				let typing = setInterval(() => {
					let now = new Date().getTime();
					if (timeStart + typing_duration < now) {
						if (record_stats) {
							console.log('tilestats: recording ended..');
							record_stats = false;
							dumpStats();
						}
						clearInterval(typing);
						console.log('End typing simulation');
						process.exit(0);
					}
//					console.debug('sending input text= ' + dummyInput[inputIndex]);
					if (dummyInput.charCodeAt(inputIndex) === 32) { // space
						window.app.socket._doSend(
							'key' +
							' type=' + 'input' +
							' char=' + dummyInput.charCodeAt(inputIndex) + ' key=0\n'
						);
					} else {
						window.app.socket._doSend(`textinput id=0 text=${dummyInput[inputIndex]}`);
					}
					inputIndex = (inputIndex + 1) % dummyInput.length;
				}, typing_speed);
			}
			else
				console.debug('No bookmark to jump to');
		}, 500);
	});
};
