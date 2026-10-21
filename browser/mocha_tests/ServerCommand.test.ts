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

/// all tests based on the possible messages from wsd/protocol.txt
describe('ServerCommand', function () {
	const mapzoom: MapZoomInterface = {
		_docLayer: {
			options: {
				tileWidthTwips: 256 * 15,
			},
		},
		options: {
			zoom: 10,
		},
	};

	it('downloadas', function () {
		const downloadid =
			'SWhzgceoAoUXjc6XxSfKduleBU3pFKPI5jUQFnmRsgRxAPMqOhI8hhY0uWmg6tSp';
		const port = '9980';
		const id = 'export';

		const msg =
			'downloadas: downloadid=' + downloadid + ' port=' + port + ' id=' + id;
		const sc = new ServerCommand(msg, mapzoom);
		nodeassert.equal(downloadid, sc.downloadid);
		nodeassert.equal(port, sc.port);
		nodeassert.equal(id, sc.id);
	});

	it('getchildid', function () {
		const id = '23456';
		const msg = 'getchildid: id=' + id;
		const sc = new ServerCommand(msg, mapzoom);
		nodeassert.equal(id, sc.id);
	});

	it('invalidatetiles (specific region)', function () {
		const part = '{1BE1A269-4A03-4202-ACFE-0204C5E9BE1F}';
		const mode = 0;
		const x = 117690;
		const y = 256050;
		const width = 2100;
		const height = 2655;
		const wireId = '268';

		const msg = `invalidatetiles: part=${part} mode=${mode} x=${x} y=${y} width=${width} height=${height} wid=${wireId}`;

		const sc = new ServerCommand(msg, mapzoom);
		nodeassert.equal(part, sc.part);
		nodeassert.equal(mode, sc.mode);
		nodeassert.equal(x, sc.x);
		nodeassert.equal(y, sc.y);
		nodeassert.equal(width, sc.width);
		nodeassert.equal(height, sc.height);
		nodeassert.equal(wireId, sc.wireId);
	});

	it('invalidatetiles (all cached tiles)', function () {
		const part = '{1BE1A269-4A03-4202-ACFE-0204C5E9BE1F}';
		const mode = 0;
		const wireId = '268';

		const msg = `invalidatetiles: EMPTY, ${part}, ${mode}, wid=${wireId}`;

		const sc = new ServerCommand(msg, mapzoom);
		nodeassert.equal(wireId, sc.wireId);
	});

	it('loaded', function () {
		const viewid = '34';
		const views = 39;
		const isfirst = false;

		const msg = `loaded: viewid=${viewid} views=${views} isfirst=${isfirst}`;
		const sc = new ServerCommand(msg, mapzoom);
		nodeassert.equal(viewid, sc.viewid);
	});

	it('pong', function () {
		const rendercount = 546;
		const msg = `pong rendercount=${rendercount}`;

		const sc = new ServerCommand(msg, mapzoom);
		nodeassert.equal(rendercount, sc.rendercount);
	});

	it('tile', function () {
		const nviewid = '1000';
		const part = '0';
		const width = 256;
		const height = 256;
		const tileposx = 122880;
		const tileposy = 215040;
		const tilewidth = 3840;
		const tileheight = 3840;
		const wireId = '423';
		const ver = '128';
		const imgsize = 800;

		const msg = `tile: nviewid=${nviewid} part=${part} width=${width} height=${height} tileposx=${tileposx} tileposy=${tileposy} tilewidth=${tilewidth} tileheight=${tileheight} wid=${wireId} ver=${ver} imgsize=${imgsize}`;

		const sc = new ServerCommand(msg, mapzoom);
		nodeassert.equal(nviewid, sc.nviewid);
		nodeassert.equal(part, sc.part);
		nodeassert.equal(width, sc.width);
		nodeassert.equal(height, sc.height);
		nodeassert.equal(tileposx, sc.x);
		nodeassert.equal(tileposy, sc.y);
		nodeassert.equal(tilewidth, sc.tileWidth);
		nodeassert.equal(tileheight, sc.tileHeight);
		nodeassert.equal(wireId, sc.wireId);
	});
});
