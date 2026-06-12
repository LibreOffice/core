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
 * Admin Server Audit tab: the instance-wide server audit results.
 */

/* global Admin AdminSocketBase _ */
var AdminSocketServerAudit = AdminSocketBase.extend({
	constructor: function (host) {
		this.base(host);
	},

	onSocketOpen: function () {
		// Authenticate first.
		this.base.call(this);
		this.socket.send('server_audit');
		this.socket.send('subscribe server_audit');
	},

	// code -> status -> human-readable detail. The strings are kept in sync
	// with errorCodes in browser/src/control/Control.ServerAuditDialog.ts,
	// which is the canonical list. Only the instance-wide codes appear here;
	// per-user codes are not aggregated into this view.
	_details: {
		certwarning: {
			sslverifyfail: _(
				'Your WOPI server is not secure: SSL verification failed',
			),
		},
		contained: {
			uncontained: _(
				'Documents are not effectively contained: missing capabilities or namespaces.',
			),
		},
		seccomp: {
			none: _(
				'BPF filtering of potentially risky system calls (seccomp) is not enabled; a security hazard.',
			),
		},
		hardwarewarning: {
			lowresources: _(
				'Your server is configured with insufficient hardware resources, which may lead to poor performance.',
			),
		},
		bindmounted: {
			slow: _('Slow Kit jail setup with copying, cannot bind-mount.'),
			not_recommended: _('Fast kit jail bind mounting not configured'),
		},
		wsurl: {
			not_recommended: _(
				'Some clients connected with the old WebSocket URL form, which is planned for removal. Check that your proxy passes the compact form through.',
			),
		},
	},

	// seccomp and contained have no SDK page in the in-window dialog, so they
	// carry no link here either.
	_links: {
		certwarning:
			'https://sdk.collaboraonline.com/docs/installation/Configuration.html#ssl-configuration',
		hardwarewarning:
			'https://sdk.collaboraonline.com/docs/installation/Configuration.html#performance',
		bindmounted:
			'https://sdk.collaboraonline.com/docs/installation/Configuration.html#performance',
		wsurl:
			'https://sdk.collaboraonline.com/docs/installation/Proxy_settings.html',
	},

	_render: function (entries) {
		var body = document.getElementById('server-audit-body');
		var empty = document.getElementById('server-audit-empty');
		body.replaceChildren();

		var issues = entries.filter(function (entry) {
			return entry.status !== 'ok';
		});
		empty.style.display = issues.length ? 'none' : '';

		issues.forEach(function (entry) {
			var isWarning = entry.status === 'not_recommended';

			var status = document.createElement('td');
			status.textContent = isWarning ? _('Warning') : _('Error');
			status.className = isWarning ? 'has-text-warning' : 'has-text-danger';

			var detail = document.createElement('td');
			var byStatus = this._details[entry.code];
			var text =
				(byStatus && byStatus[entry.status]) ||
				entry.code + ': ' + entry.status;
			detail.appendChild(document.createTextNode(text + ' '));

			var link = this._links[entry.code];
			if (link) {
				var anchor = document.createElement('a');
				anchor.href = link;
				anchor.target = '_blank';
				anchor.rel = 'noopener';
				anchor.textContent = _('Learn more');
				detail.appendChild(anchor);
			}

			var row = document.createElement('tr');
			row.appendChild(status);
			row.appendChild(detail);
			body.appendChild(row);
		}, this);
	},

	onSocketMessage: function (e) {
		var textMsg = typeof e.data === 'string' ? e.data : '';
		if (textMsg.startsWith('server_audit')) {
			var json = JSON.parse(textMsg.substring('server_audit'.length));
			this._render(json.serverAudit || []);
		}
	},

	onSocketClose: function () {
		this.base.call(this);
	},
});

Admin.ServerAudit = function (host) {
	return new AdminSocketServerAudit(host);
};
