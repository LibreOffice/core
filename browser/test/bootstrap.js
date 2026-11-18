/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
const https = require("https");
const http = require("http");

const { spawn, fork } = require('child_process');
if (process.argv.length < 4 || process.argv[2] == '--help') {
	console.debug('bootstrap.js <abs_top_builddir> <abs_srcdir>');
	process.exit(0);
}

// Websocket can't cope with SSL certificates.
const ssl_flag = false;

const top_builddir = process.argv[2];
const srcdir = process.argv[3];
const typing_speed = process.argv[4];
const single_view = process.argv[5];
const typing_duration = process.argv[6];
const inspect = process.argv[7];
const recordStats = process.argv[8];

// verbose console output
const debug = false;

/* don't use the default port (9980) */
const port = '9999';
let args = [
	`--o:sys_template_path=${top_builddir}/systemplate`,
	`--o:child_root_path=${top_builddir}/jails`,
	`--o:cache_files.path=${top_builddir}/cache`,
	'--o:storage.filesystem[@allow]=true',
	'--o:admin_console.username=admin',
	'--o:admin_console.password=admin',
	'--o:logging.file[@enable]=true --o:logging.level=' + (debug ? 'trace' : 'warning'),
	'--o:trace_event[@enable]=true',
	`--port=${port}`,
	'--signal'
];

let ssl_args = [
	`--o:ssl.cert_file_path=${top_builddir}/etc/cert.pem`,
	`--o:ssl.key_file_path=${top_builddir}/etc/key.pem`,
	`--o:ssl.ca_file_path=${top_builddir}/etc/ca-chain.cert.pem`,
];

if (ssl_flag === 'true')
	args = [...args, ...ssl_args];
else
	args = [...args, '--o:ssl.enable=false'];

process.on('SIGUSR2', serverReady);

var coolwsd_options = debug ? {} : { stdio: 'ignore'};
const coolwsd = spawn(`${top_builddir}/coolwsd`, args, coolwsd_options);

if (debug)
{
	coolwsd.stdout.on('data', (data) => {
		console.log(`stdout: ${data}`);
	});
	coolwsd.stderr.on('data', (data) => {
		console.error(`stderr: ${data}`);
	});
}

coolwsd.on('exit', (code) => {
	console.log(`coolwsd process exited with code ${code}`);
});

process.env.NODE_PATH = `${top_builddir}/browser/node_modules`
let childNodes = [];

function serverReady() {
    console.log('\nTest running - connect to:\n\n\t' +
		(ssl_flag === 'true'?'https':'http') +
		'://localhost:9999/browser/1234/cool.html?file_path=file://' +
		top_builddir + '/test/data/perf-test-edit.odt\n\n');

    let execArgs = [];

    if (inspect === 'true')
	execArgs.push('--inspect-brk');
    childNodes.push(
	fork(`${srcdir}/test/load.js`, [ssl_flag, top_builddir, `${top_builddir}/test/data/perf-test-edit.odt`, `testEdit_1`, `${port}`, `${typing_speed}`, `${typing_duration}`, `${recordStats}`, `${single_view}`], {execArgv: execArgs})
);
    if(single_view !== "true") {
	for (let i = 2; i <= 6; i++) {
	    childNodes.push(
		fork(`${srcdir}/test/load.js`, [ssl_flag, top_builddir, `${top_builddir}/test/data/perf-test-edit.odt`, `testEdit_${i}`, `${port}`, `${typing_speed}`, `${typing_duration}`, 'false', 'false'])
		);
	}
    }
    setInterval(dumpMemoryUse, 3000);
}

function vacuumCleaner(kill, message, code) {
		console.log(message);
		childNodes.forEach(n => n.kill(kill));
		coolwsd.kill(kill);
		console.log(`Process exited with code ${code}`);
}

function exitHandler(options, exitCode) {
	if (options.cleanup) {
		vacuumCleaner('SIGKILL', 'cleaning up...', exitCode)
	}
	if (options.exit) {
		vacuumCleaner('SIGINT', 'exiting...', exitCode)
	}
	process.exit();
}

//do something when app is closing
process.on('exit', exitHandler.bind(null,{cleanup: true}));

//catches ctrl+c event
process.on('SIGINT', exitHandler.bind(null, {exit: true}));

//catches uncaught exceptions
process.on('uncaughtException', ex => {
	console.error(ex, 'uncaught exception');
	exitHandler({exit:true});
});

function parseStats(content) {
	var stats = {};
	var lines = content.split('\n');
	if (content.length < 128 || lines.length < 16)
		return undefined; // too small

	for (let l of lines) {
		var keyval = l.split(' ');
		if (keyval.length >= 2)
			stats[keyval[0]] = Number(keyval[1]);
	}
	if (stats.size < 8)
		return undefined; // not our stats

	return stats;
}

function getHttpProtocol() {
    return ssl_flag === 'true' ? https : http;
}

function dumpMemoryUse() {
	var url = (ssl_flag === 'true' ? 'https' : 'http') + '://admin:admin@localhost:' + port + '/cool/getMetrics/';
	console.log('Fetching stats from ' + url);
	var req = getHttpProtocol().request(
		url,
		{
			rejectUnauthorized: false,
			requestCert: false,
			timeout: 3000, // 3s
		},
		response => {
			let data = [];
			response.on('data', (frag) => {
				data.push(frag);
			});
			response.on('end', () => {
				let body = Buffer.concat(data);
				var stats = parseStats(body.toString());
				if (stats)
					console.log('Stats: ' +
						    'views: ' + stats['document_all_views_all_count_total'] + ' ' +
						    'mem: ' + (stats['global_memory_used_bytes']/1000000) + 'Mb ' +
						    'sent: ' + (stats['document_all_sent_to_clients_total_bytes']/1000000) + 'Mb ' +
						    'recv: ' + (stats['document_all_received_from_clients_total_bytes']/1000) + 'Kb');
			});
			response.on('error', (err) => {
				console.log('failed to get admin stats');
			});
		});
	req.end();
}
