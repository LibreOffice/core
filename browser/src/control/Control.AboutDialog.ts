// @ts-strict-ignore
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
 * AboutDialog - implements Help - About dialog with version and warnings
 */

declare var JSDialog: any;
declare var brandProductName: any;
declare var sanitizeUrl: any;

interface AboutDialogElements {
	coolwsdVersion: HTMLElement;
	servedBy: HTMLElement;
	slowProxy: HTMLElement;
	jsDialog: HTMLElement;
	routeToken: HTMLElement;
	timeZone: HTMLElement;
	wopiHostId: HTMLElement;
	licenseInfo: HTMLElement;
	copyright: HTMLElement;
}

class AboutDialog {
	map: any;

	constructor(map: any) {
		this.map = map;
	}

	private aboutDialogClickHandler(e: any) {
		if (e.detail === 3) {
			this.map._debug.toggle();
		}
	}

	private adjustIDs(content: HTMLElement) {
		const servedBy = content.querySelector(':scope #served-by');
		if (servedBy) servedBy.id += '-cloned';

		const wopiHostId = content.querySelector(':scope #wopi-host-id');
		if (wopiHostId) wopiHostId.id += '-cloned';
	}

	private hideElementsHiddenByDefault(content: HTMLElement) {
		const servedBy = content.querySelector(
			':scope #served-by-cloned',
		) as HTMLElement;
		if (servedBy) servedBy.style.display = 'none';

		const wopiHostId = content.querySelector(
			':scope #wopi-host-id-cloned',
		) as HTMLElement;
		if (wopiHostId) wopiHostId.style.display = 'none';
	}

	private static appendSpanAndLink(
		target: HTMLElement,
		labelText: string,
		linkHref: string,
		linkText: string,
		extraText?: string,
		spanId?: string,
	): void {
		const span = document.createElement('span');
		if (spanId) {
			span.id = spanId;
		}

		span.appendChild(document.createTextNode(labelText));

		const a = document.createElement('a');
		a.href = linkHref;
		a.target = '_blank';
		a.textContent = linkText;
		// WCAG 3.2.5: without altering what sighted users see.
		const srOnly = document.createElement('span');
		srOnly.className = 'visuallyhidden';
		srOnly.textContent = ' ' + _('(opens in new tab)');
		a.appendChild(srOnly);

		span.appendChild(a);
		if (extraText) {
			span.appendChild(document.createTextNode(`\xA0${extraText}`));
		}

		target.appendChild(span);
	}

	// This method can be used in the future to populate the about dialog content
	private static populateAboutDialog(
		content: HTMLElement,
		elements: AboutDialogElements,
	): void {
		const info = window.app.serverInfo;

		if (!info) {
			app.console.error('AboutDialog: serverInfo is not available');
			return;
		}

		// fill product-name
		let productName;
		if (window.ThisIsAMobileApp) {
			productName = window.MobileAppName;
		} else {
			productName =
				typeof brandProductName === 'string' && brandProductName.length > 0
					? brandProductName
					: 'Collabora Online Development Edition (unbranded)';
		}

		const productNameElement = content.querySelector(
			'#product-name',
		) as HTMLElement;
		productNameElement.innerText = productName;
		content.classList.add(
			'product-' +
				productName
					.split(/[ ()]+/)
					.join('-')
					.toLowerCase(),
		);

		// COOLWSD version
		elements.coolwsdVersion.textContent = info.coolwsdVersion;
		this.appendSpanAndLink(
			elements.coolwsdVersion,
			' git hash:\xA0',
			`https://gerrit.collaboraoffice.com/plugins/gitiles/online/+log/${info.coolwsdHash}`,
			info.coolwsdHash,
			info.wsdOptions,
		);

		// Served By and Server ID
		const label = document.createElement('span');
		label.id = 'served-by-label';
		label.textContent = _('Served by:\xA0');
		elements.servedBy.appendChild(label);

		// OS Info
		const os = document.createElement('span');
		os.textContent = info.osInfo;
		elements.servedBy.appendChild(os);
		elements.servedBy.appendChild(document.createElement('wbr'));
		elements.servedBy.appendChild(document.createTextNode('\xA0'));
		this.appendSpanAndLink(
			elements.servedBy,
			'',
			window.makeHttpUrl('/hosting/discovery'),
			info.serverId,
			'',
			'coolwsd-id',
		);

		// JSDialogs link for debug mode
		if (window.enableDebug) {
			const label = document.createTextNode('JSDialogs:\xA0');
			const link = document.createElement('a');
			link.href = '#';
			link.textContent = _(`View widgets`);

			link.addEventListener('click', (e: MouseEvent) => {
				e.preventDefault();
				app.socket.sendMessage('uno .uno:WidgetTestDialog');
				app.map.uiManager.closeModal('modal-dialog-about-dialog-box');
			});

			elements.jsDialog.appendChild(label);
			elements.jsDialog.appendChild(link);
		}

		// WOPI Host ID
		elements.wopiHostId.textContent = window.wopiHostId;

		// License information (apps only)
		if (window.ThisIsAMobileApp) {
			const licenseLink = document.createElement('a');
			licenseLink.href = 'javascript:void(0)';
			licenseLink.textContent = _UNO('.uno:ShowLicense');
			licenseLink.addEventListener('click', () =>
				window.postMobileMessage('LICENSE'),
			);
			elements.licenseInfo.appendChild(licenseLink);
		}

		// Copyright and vendor
		const span = document.createElement('span');
		span.setAttribute('dir', 'ltr');
		span.textContent = _(
			`Copyright © ${window.copyrightYear}, ${window.vendor}.`,
		);
		elements.copyright.appendChild(span);

		if (window.socketProxy) {
			elements.slowProxy.innerText = _('"Slow Proxy"');
		}

		if (window.indirectSocket) {
			elements.routeToken.innerText = 'RouteToken: ' + window.routeToken;
			if (window.geolocationSetup) {
				elements.timeZone.innerText =
					_('Time zone:') + ' ' + app.socket.WSDServer.TimeZone;
			}
		}
	}

	public show() {
		const aboutDialogId = 'about-dialog';

		const content: HTMLElement = document
			.getElementById(aboutDialogId)
			.cloneNode(true) as HTMLElement;
		content.style.display = 'block';
		const elements = AboutDialog.createAboutDialogContent(content);

		AboutDialog.populateAboutDialog(content, elements);

		this.adjustIDs(content);
		this.hideElementsHiddenByDefault(content); // Now we can safely hide the elements that we want hidden by default.

		const productName =
			content.querySelector('#product-name').textContent || '';
		this.map.uiManager.showYesNoButton(
			aboutDialogId + '-box',
			productName,
			'',
			_('OK'),
			null,
			null,
			null,
			true,
		);

		this.showImpl(aboutDialogId, content);
	}

	showImpl(aboutDialogId: string, content: HTMLElement) {
		var box = document.getElementById(aboutDialogId + '-box');

		// TODO: do it JSDialog native...
		if (!box) {
			setTimeout(() => {
				this.showImpl(aboutDialogId, content);
			}, 10);
			return;
		}

		var innerDiv = window.L.DomUtil.create('div', '', null);
		box.insertBefore(innerDiv, box.firstChild);
		innerDiv.appendChild(content);

		var form = document.getElementById('about-dialog-box');

		form.addEventListener('click', this.aboutDialogClickHandler.bind(this));
		form.addEventListener('keyup', this.aboutDialogKeyHandler.bind(this));
		form.querySelector('#coolwsd-version').querySelector('a').focus();
		const copyVersionText = _('Copy all version information in English');
		var copyVersion = window.L.DomUtil.create(
			'button',
			'ui-pushbutton jsdialog',
			null,
		);
		copyVersion.setAttribute('id', 'modal-dialog-about-dialog-box-copybutton');
		copyVersion.setAttribute('aria-label', copyVersionText);
		copyVersion.setAttribute('data-cooltip', copyVersionText);
		var img = window.L.DomUtil.create('img', null, null);
		app.LOUtil.setImage(img, 'lc_copy.svg', this.map);
		copyVersion.innerHTML =
			'<img src="' + sanitizeUrl(img.src) + '" width="18px" height="18px">';
		copyVersion.addEventListener(
			'click',
			this.copyVersionInfoToClipboard.bind(this),
		);
		window.L.control.attachTooltipEventListener(copyVersion, this.map);
		var aboutOk = document.getElementById(
			'modal-dialog-about-dialog-box-yesbutton',
		);
		if (aboutOk) {
			aboutOk.before(copyVersion);
		}
	}

	private aboutDialogKeyHandler(e: KeyboardEvent) {
		if (e.key === 'd') {
			this.map._debug.toggle();
		} else if (e.key === 'l') {
			// L toggles the Online logging level between the default (whatever
			// is set in coolwsd.xml or on the coolwsd command line) and the
			// most verbose a client is allowed to set (which also can be set in
			// coolwsd.xml or on the coolwsd command line).
			//
			// In a typical developer "make run" setup, the default is "trace"
			// so there is nothing more verbose. But presumably it is different
			// in production setups.

			app.socket.threadLocalLoggingLevelToggle =
				!app.socket.threadLocalLoggingLevelToggle;

			const newLogLevel = app.socket.threadLocalLoggingLevelToggle
				? 'verbose'
				: 'default';
			app.socket.sendMessage('loggingleveloverride ' + newLogLevel);

			const logLevelInformation = app.socket.threadLocalLoggingLevelToggle
				? 'most verbose (from coolwsd.xml)'
				: 'default (from coolwsd.xml)';
			console.debug('Log level: ' + logLevelInformation);
		}
	}

	private copyVersionInfoToClipboard() {
		const info = window.app.serverInfo;
		let text = '';

		if (!info) {
			app.console.error('ServerInfo is not available');
			return;
		}

		const addLine = (label: string, value: string) => {
			if (value && value.trim()) {
				text += `${label}: ${value.trim()}\n`;
			}
		};

		let coolwsdLine = info.coolwsdVersion;
		coolwsdLine += ` (git hash: ${info.coolwsdHash} ${info.wsdOptions})`;
		addLine('COOLWSD version', coolwsdLine);

		addLine('Served by', info.osInfo);
		addLine('Server ID', info.serverId);
		addLine('WOPI host', window.wopiHostId);

		text = text.replace(/\u00A0/g, ' ');

		if (window.mode.isCODesktop()) {
			(window as any).postMobileMessage('TEXTCLIPBOARD ' + text);
			this.contentHasBeenCopiedShowSnackbar();
		} else if (navigator.clipboard && window.isSecureContext) {
			navigator.clipboard
				.writeText(text)
				.then(
					function () {
						window.console.log('Text copied to clipboard');
						this.contentHasBeenCopiedShowSnackbar();
					}.bind(this),
				)
				.catch(function (error) {
					window.console.error('Error copying text to clipboard:', error);
				});
		} else {
			var textArea = document.createElement('textarea');
			textArea.style.position = 'absolute';
			textArea.style.opacity = '0';
			textArea.value = text;
			document.body.appendChild(textArea);
			textArea.select();
			try {
				document.execCommand('copy');
				window.console.log('Text copied to clipboard');
				this.contentHasBeenCopiedShowSnackbar();
			} catch (error) {
				window.console.error('Error copying text to clipboard:', error);
			} finally {
				document.body.removeChild(textArea);
			}
		}
	}

	private contentHasBeenCopiedShowSnackbar() {
		const timeout = 1000;
		this.map.uiManager.showSnackbar(
			_('Version information has been copied'),
			null,
			null,
			timeout,
		);
		const copybutton = document.querySelector(
			'#modal-dialog-about-dialog-box-copybutton > img',
		) as HTMLImageElement;
		app.LOUtil.setImage(copybutton, 'lc_clipboard-check.svg', this.map);
		setTimeout(() => {
			app.LOUtil.setImage(copybutton, 'lc_copy.svg', this.map);
		}, timeout);
	}

	private static createElement(
		tag: string,
		options: {
			id?: string;
			className?: string;
			attrs?: Record<string, string>;
		} = {},
	): HTMLElement {
		const el = document.createElement(tag);
		if (options.id) el.id = options.id;
		if (options.className) el.className = options.className;
		if (options.attrs) {
			Object.entries(options.attrs).forEach(([k, v]) => el.setAttribute(k, v));
		}
		return el;
	}

	// Shared static method to create the About dialog structure
	public static createAboutDialogContent(
		content: HTMLElement,
	): AboutDialogElements {
		const infoDiv = content.querySelector('#about-dialog-info') as HTMLElement;

		const coolwsdLabel = AboutDialog.createElement('div', {
			id: 'coolwsd-version-label',
		});
		coolwsdLabel.textContent = _('COOLWSD version:');
		infoDiv.appendChild(coolwsdLabel);

		const coolwsdVersionContainer = AboutDialog.createElement('div', {
			className: 'about-dialog-info-div',
		});
		const coolwsdVersion = AboutDialog.createElement('div', {
			id: 'coolwsd-version',
			attrs: { dir: 'ltr' },
		});
		coolwsdVersionContainer.appendChild(coolwsdVersion);
		infoDiv.appendChild(coolwsdVersionContainer);

		const servedBy = AboutDialog.createElement('div', { id: 'served-by' });
		if (!window.ThisIsAMobileApp) {
			infoDiv.appendChild(servedBy);
		}

		const slowProxy = AboutDialog.createElement('div', { id: 'slow-proxy' });
		infoDiv.appendChild(slowProxy);

		const jsDialog = AboutDialog.createElement('div', { id: 'js-dialog' });
		if (window.enableDebug) {
			infoDiv.appendChild(jsDialog);
		}

		const routeToken = AboutDialog.createElement('div', { id: 'routeToken' });
		infoDiv.appendChild(routeToken);

		const timeZone = AboutDialog.createElement('div', { id: 'timeZone' });
		infoDiv.appendChild(timeZone);

		const wopiHostId = AboutDialog.createElement('div', { id: 'wopi-host-id' });
		if (!window.ThisIsAMobileApp) {
			infoDiv.appendChild(wopiHostId);
		}

		// License information (apps only)
		const licenseInfo = AboutDialog.createElement('div', {
			id: 'license-information',
		});
		if (window.ThisIsAMobileApp) {
			infoDiv.appendChild(licenseInfo);
		}

		// Copyright
		const copyright = AboutDialog.createElement('p', {
			className: 'about-dialog-info-div',
		});
		infoDiv.appendChild(copyright);

		return {
			coolwsdVersion,
			servedBy,
			slowProxy,
			jsDialog,
			routeToken,
			timeZone,
			wopiHostId,
			licenseInfo,
			copyright,
		};
	}
}

// Initiate the class.
JSDialog.aboutDialog = (map: any) => {
	return new AboutDialog(map);
};
