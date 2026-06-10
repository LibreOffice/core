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
 * ServerAuditDialog - Dialog for admin users showing checklist of server security settings
 */

declare var JSDialog: any;

interface AuditEntry {
	code: string;
	status: string;
}

class ClientAuditor {
	private static checkPostMessages(entries: Array<AuditEntry>) {
		if ((window as any).WOPIPostmessageReady)
			entries.push({ code: 'postmessage', status: 'ok' });
		else entries.push({ code: 'postmessage', status: 'hostnotready' });
	}

	private static checkProxyProtocol(entries: Array<AuditEntry>) {
		if (window.socketProxy) entries.push({ code: 'proxy', status: 'slow' });
		else entries.push({ code: 'proxy', status: 'ok' });
	}

	public static performClientAudit(): Array<AuditEntry> {
		const entries = new Array<AuditEntry>();
		ClientAuditor.checkPostMessages(entries);
		ClientAuditor.checkProxyProtocol(entries);
		return entries;
	}
}

class ServerAuditDialog {
	map: any;
	id: string = 'ServerAuditDialog';
	errorCodes: any;

	constructor(map: any) {
		this.map = map;
		this.map.on('receivedserveraudit', this.onServerAudit.bind(this), this);

		// Priorities: 1 - security, 10 - config, 20 - integration, 30 - general info ...
		this.errorCodes = {
			certwarning: {
				priority: 1,
				sslverifyfail: [
					_('Your WOPI server is not secure: SSL verification failed'),
					'SDK: ssl-configuration',
					'https://sdk.collaboraonline.com/docs/installation/Configuration.html?highlight=ssl#ssl-configuration',
				],
				ok: [
					_('No problems with SSL verification detected'),
					'SDK: ssl-configuration',
					'https://sdk.collaboraonline.com/docs/installation/Configuration.html?highlight=ssl#ssl-configuration',
				],
			},
			contained: {
				priority: 2,
				uncontained: [
					_(
						'Documents are not effectively contained: missing capabilities or namespaces.',
					),
					'SDK: nocaps',
					'',
				],
				ok: [_('Each document is securely contained'), 'SDK: nocaps', ''],
			},
			seccomp: {
				priority: 3,
				none: [
					_(
						'BPF filtering of potentially risky system calls (seccomp) is not enabled; a security hazard.',
					),
					'SDK: seccomp',
					'',
				],
				ok: [_('System call security filtering enabled'), 'SDK: seccomp', ''],
			},

			hardwarewarning: {
				priority: 10,
				lowresources: [
					_(
						'Your server is configured with insufficient hardware resources, which may lead to poor performance.',
					),
					'SDK: hardware-requirements',
					'https://sdk.collaboraonline.com/docs/installation/Configuration.html#performance',
				],
				ok: [
					_('Hardware resources are sufficient for optimal performance'),
					'SDK: hardware-requirements',
					'https://sdk.collaboraonline.com/docs/installation/Configuration.html#performance',
				],
			},
			bindmounted: {
				priority: 11,
				slow: [
					_('Slow Kit jail setup with copying, cannot bind-mount.'),
					'SDK: bindmount',
					'https://sdk.collaboraonline.com/docs/installation/Configuration.html#performance',
				],
				ok: [
					_('Fast kit jail bind mounting enabled'),
					'SDK: bindmount',
					'https://sdk.collaboraonline.com/docs/installation/Configuration.html#performance',
				],
				not_recommended: [
					_('Fast kit jail bind mounting not configured'),
					'SDK: bindmount',
					'https://sdk.collaboraonline.com/docs/installation/Configuration.html#performance',
				],
			},
			proxy: {
				priority: 12,
				slow: [
					_('Poorly performing proxying of all network requests.'),
					'',
					'',
				],
				ok: [_('Direct network connection'), '', ''],
			},
			wsurl: {
				priority: 13,
				not_recommended: [
					_(
						'Connected with the old WebSocket URL form, which is planned for removal. Check that your proxy passes the compact form through.',
					),
					'SDK: proxy-settings',
					'https://sdk.collaboraonline.com/docs/installation/Proxy_settings.html',
				],
				ok: [
					_('Connected with the compact WebSocket URL form'),
					'SDK: proxy-settings',
					'https://sdk.collaboraonline.com/docs/installation/Proxy_settings.html',
				],
			},

			is_admin: {
				priority: 20,
				missing: [
					_('The IsAdminUser property is not set by integration'),
					'SDK: IsAdminUser',
					'https://sdk.collaboraonline.com/docs/advanced_integration.html?highlight=IsAdminUser#isadminuser',
				],
				deprecated: [
					_(
						'Used deprecated is_admin field, integration should use IsAdminUser property instead',
					),
					'SDK: IsAdminUser',
					'https://sdk.collaboraonline.com/docs/advanced_integration.html?highlight=IsAdminUser#isadminuser',
				],
				ok: [
					_('The IsAdminUser user property is set by integration'),
					'SDK: IsAdminUser',
					'https://sdk.collaboraonline.com/docs/advanced_integration.html?highlight=IsAdminUser#isadminuser',
				],
			},
			postmessage: {
				priority: 21,
				ok: [
					_('PostMessage API is initialized'),
					'SDK: post-message-initialization',
					'https://sdk.collaboraonline.com/docs/postmessage_api.html#initialization',
				],
				hostnotready: [
					_('Integrator is not ready for PostMessage calls'),
					'SDK: post-message-initialization',
					'https://sdk.collaboraonline.com/docs/postmessage_api.html#initialization',
				],
			},

			info_namespaces: {
				priority: 30,
				true: [_('Using namespaces'), 'SDK: nocaps', ''],
				false: [_('Not using namespaces'), 'SDK: nocaps', ''],
			},
		};
	}

	public open() {
		const serverEntries = this.getEntries(app.serverAudit);
		app.clientAudit = ClientAuditor.performClientAudit();
		const clientEntries = this.getEntries(app.clientAudit);
		const allEntries = serverEntries.concat(clientEntries);

		// Sort errors to the top
		allEntries.sort((a, b) => {
			const aIsError = a.columns[0].collapsed === 'serverauditerror.svg';
			const bIsError = b.columns[0].collapsed === 'serverauditerror.svg';
			if (aIsError !== bIsError) return aIsError ? -1 : 1;
			return 0; // keep internal order, already sorted by priority
		});

		const dialogBuildEvent = {
			data: this.getJSON(allEntries),
			callback: this.callback.bind(this) as JSDialogCallback,
		};

		this.map.fire(
			window.mode.isSmallScreenDevice() ? 'mobilewizard' : 'jsdialog',
			dialogBuildEvent,
		);
	}

	private getEntries(sourceUnsorted: any): Array<TreeEntryJSON> {
		const entries = new Array<TreeEntryJSON>();

		if (!sourceUnsorted) return entries;

		const errorIcon = { collapsed: 'serverauditerror.svg' };
		const warnIcon = { collapsed: 'serverauditwarn.svg' };
		const okIcon = { collapsed: 'serverauditok.svg' };

		const source = sourceUnsorted.sort(
			(x: AuditEntry, y: AuditEntry) =>
				(this.errorCodes[x.code] ? this.errorCodes[x.code].priority : 100) -
				(this.errorCodes[y.code] ? this.errorCodes[y.code].priority : 100),
		);

		source.forEach((entry: AuditEntry) => {
			const found = this.errorCodes[entry.code];
			if (found) {
				const status = found[entry.status];
				if (status) {
					entries.push({
						row: 0,
						columns: [
							!this.isErrorEntry(entry)
								? this.isWarnEntry(entry)
									? warnIcon
									: okIcon
								: errorIcon,
							{ text: status[0] },
							status[1] && status[2]
								? {
										text: status[1],
										link: status[2],
									}
								: { text: '' },
						],
					} as TreeEntryJSON);
				}
			} else if (this.isInfoEntry(entry)) {
				if (entry.code === 'info_setup_ms') {
					const ms = Number.parseInt(entry.status);
					const good = ms < 3000;
					entries.push({
						row: 0,
						columns: [
							good ? okIcon : errorIcon,
							{
								text: _('Document container started in {1} ms').replace(
									'{1}',
									entry.status,
								),
							},
							{ text: '' },
						],
					} as TreeEntryJSON);
				} else console.warn('Unknown server audit info: ' + entry.code);
			} else {
				console.warn('Unknown server audit entry: ' + entry.code);
			}
		});

		return entries;
	}

	private hasErrors(): boolean {
		let hasErrors = false;
		if (app.serverAudit) {
			app.serverAudit.forEach((entry: any) => {
				if (this.isErrorEntry(entry)) hasErrors = true;
			});
		}

		if (app.clientAudit) {
			app.clientAudit.forEach((entry: any) => {
				if (entry.status !== 'ok') hasErrors = true;
			});
		}

		return hasErrors;
	}

	private countErrors(): number {
		return (
			(app.serverAudit?.filter((entry: AuditEntry) => this.isErrorEntry(entry))
				.length ?? 0) +
			(app.clientAudit?.filter((entry: AuditEntry) => entry.status !== 'ok')
				.length ?? 0)
		);
	}

	private getJSON(entries: Array<any>): JSDialogJSON {
		const hasErrors = this.hasErrors();
		const countErrors = this.countErrors();

		return {
			id: this.id,
			dialogid: this.id,
			type: 'dialog',
			text: _('Server audit'),
			title: _('Server audit'),
			jsontype: 'dialog',
			responses: [
				{
					id: 'ok',
					response: 1,
				},
			],
			children: [
				{
					id: this.id + '-mainbox',
					type: 'container',
					vertical: true,
					children: [
						{
							id: 'auditlist',
							type: 'treelistbox',
							headers: [
								/* icon */ { text: _('Status'), sortable: false },
								{ text: _('Help'), sortable: false },
							],
							entries: entries,
							enabled: entries.length > 0,
						},
						!hasErrors
							? {
									id: 'auditsuccess',
									type: 'fixedtext',
									text: _('No issues found'),
								}
							: {
									id: 'auditerror',
									type: 'fixedtext',
									text: _('Alerts:') + ' ' + countErrors,
								},
						{
							id: this.id + '-buttonbox',
							type: 'buttonbox',
							children: [
								{
									id: 'ok',
									type: 'pushbutton',
									text: _('OK'),
								} as PushButtonWidget,
							],
							layoutstyle: 'end',
						} as ButtonBoxWidget,
					],
				} as ContainerWidgetJSON,
			],
		} as JSDialogJSON;
	}

	public close() {
		const closeEvent = {
			data: {
				action: 'close',
				id: this.id,
			},
		};
		this.map.fire(
			window.mode.isSmallScreenDevice() ? 'closemobilewizard' : 'jsdialog',
			closeEvent,
		);
	}

	private isInfoEntry(entry: AuditEntry): boolean {
		return entry.code.startsWith('info_');
	}

	/// Warning entries are marked with 'not_recommended'.
	/// This makes it flexible and allows for having a third
	/// option, besides 'ok' and !'ok', independent of info.
	private isWarnEntry(entry: AuditEntry): boolean {
		return entry.status === 'not_recommended';
	}

	private isErrorEntry(entry: AuditEntry): boolean {
		return (
			!this.isInfoEntry(entry) &&
			!this.isWarnEntry(entry) &&
			entry.status !== 'ok'
		);
	}

	private onServerAudit() {
		if (app.serverAudit.length) {
			let hasErrors = false;
			app.serverAudit.forEach((entry: any) => {
				if (this.isErrorEntry(entry)) hasErrors = true;
			});

			// only show the snackbar if there are specific warnings
			// and if the current view isadminuser
			if (hasErrors && app.isAdminUser) {
				this.map.uiManager.showSnackbar(
					_('Check warnings of your server'),
					_('OPEN'),
					this.open.bind(this),
				);
			}
		}
	}

	private callback(
		objectType: string,
		eventType: string,
		object: any,
		data: any,
		builder: any,
	) {
		if (eventType === 'response' || object.id === 'ok') this.close();
	}
}

JSDialog.serverAuditDialog = (map: any) => {
	return new ServerAuditDialog(map);
};
