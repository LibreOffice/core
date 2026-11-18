/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

namespace cool {
	export interface SignatureProvider {
		action_type: string;
		name: string;
		countryCodes: Array<string>;
	}

	export interface Country {
		// ISO 3166-1 alpha-2 code
		code: string;
		name: string;
	}

	/**
	 * Provides a dialog to select an electronic signing provider.
	 */
	export class ESignatureDialog {
		id: string = 'ESignatureDialog';

		availableCountries: Array<Country>;

		defaultCountryCode: string = '';

		defaultProviderId: string = '';

		availableProviders: Array<SignatureProvider>;

		// Providers available in the selected country
		filteredProviders: Array<SignatureProvider> = [];

		constructor(
			countries: Array<Country>,
			providers: Array<SignatureProvider>,
		) {
			// Sort the lists based on the UI name.
			this.availableCountries = countries;
			this.availableCountries.sort((a, b) => a.name.localeCompare(b.name));
			this.availableProviders = providers;
			this.availableProviders.sort((a, b) => a.name.localeCompare(b.name));
		}

		// Produces the JSDialog JSON for the provider listbox
		getProviderLbJSON(
			providers: Array<string>,
			defaultProviderIndex: number,
		): ListBoxWidget {
			return {
				id: 'providerlb',
				type: 'listbox',
				enabled: true,
				children: [
					{
						id: '',
						type: 'control',
						enabled: true,
						children: [],
					},
					{
						type: 'pushbutton',
						enabled: true,
						symbol: 'SPIN_DOWN',
					} as PushButtonWidget,
				],
				labelledBy: 'providerft',
				entries: providers,
				selectedCount: 1,
				selectedEntries: [String(defaultProviderIndex)],
			} as ListBoxWidget;
		}

		getChildrenJSON(
			countries: Array<string>,
			defaultCountryIndex: number,
			providers: Array<string>,
			defaultProviderIndex: number,
		): Array<WidgetJSON> {
			return [
				{
					id: 'countryft',
					type: 'fixedtext',
					text: _('Country:'),
					enabled: true,
					labelFor: 'countrylb',
				} as TextWidget,
				{
					id: 'countrylb',
					type: 'listbox',
					enabled: true,
					children: [
						{
							id: '',
							type: 'control',
							enabled: true,
							children: [],
						},
						{
							type: 'pushbutton',
							enabled: true,
							symbol: 'SPIN_DOWN',
						} as PushButtonWidget,
					],
					labelledBy: 'countryft',
					entries: countries,
					selectedCount: 1,
					selectedEntries: [String(defaultCountryIndex)],
				} as ListBoxWidget,
				{
					id: 'providerft',
					type: 'fixedtext',
					text: _('Provider:'),
					enabled: true,
					labelFor: 'providerlb',
				} as TextWidget,
				this.getProviderLbJSON(providers, defaultProviderIndex),
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
			];
		}

		// Updates the filtered provider list based on a country code and gets a default
		// provider index.
		getDefaultProviderIndex(): number {
			this.filteredProviders = this.availableProviders.filter((provider) =>
				provider.countryCodes.includes(this.defaultCountryCode),
			);
			let defaultProviderIndex = this.filteredProviders
				.map((entry) => entry.action_type)
				.indexOf(this.defaultProviderId);
			if (defaultProviderIndex == -1) {
				defaultProviderIndex = 0;
			}
			return defaultProviderIndex;
		}

		getJSON(): JSDialogJSON {
			const countries = this.availableCountries.map((entry) => entry.name);
			let defaultCountryIndex = this.availableCountries
				.map((entry) => entry.code)
				.indexOf(this.defaultCountryCode);
			if (defaultCountryIndex == -1) {
				defaultCountryIndex = 0;
			}
			const defaultProviderIndex = this.getDefaultProviderIndex();
			const providers = this.filteredProviders.map((entry) => entry.name);
			const children = this.getChildrenJSON(
				countries,
				defaultCountryIndex,
				providers,
				defaultProviderIndex,
			);
			return {
				id: this.id,
				dialogid: this.id,
				type: 'dialog',
				text: _('Insert Electronic Signature'),
				title: _('Insert Electronic Signature'),
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
						children: children,
					} as ContainerWidgetJSON,
				],
			} as JSDialogJSON;
		}

		public close(): void {
			const closeEvent = {
				data: {
					action: 'close',
					id: this.id,
				},
			};
			app.map.fire('jsdialog', closeEvent);
		}

		private callback(
			objectType: string,
			eventType: string,
			object: any,
			data: any,
			builder: JSBuilder,
		) {
			if (eventType === 'response' || object.id === 'ok') {
				const providers = <HTMLSelectElement>(
					document.querySelector('#ESignatureDialog select#providerlb-input')
				);
				const providerId =
					this.filteredProviders[providers.selectedIndex].action_type;
				const countries = <HTMLSelectElement>(
					document.querySelector('#ESignatureDialog select#countrylb-input')
				);
				const countryCode =
					this.availableCountries[countries.selectedIndex].code;
				this.close();
				Util.ensureValue(app.map.eSignature);
				app.map.eSignature.handleSelectedProvider(countryCode, providerId);
			} else if (eventType === 'selected' && object.id === 'countrylb') {
				// The selected country changed, update the list of providers
				// accordingly
				// Index-label pair
				const countryIndex = parseInt(data.split(';')[0]);
				this.defaultCountryCode = this.availableCountries[countryIndex].code;
				const defaultProviderIndex = this.getDefaultProviderIndex();
				const providers = this.filteredProviders.map((entry) => entry.name);
				const providerLbJSON = this.getProviderLbJSON(
					providers,
					defaultProviderIndex,
				);
				app.map.fire('jsdialogupdate', {
					data: {
						jsontype: 'dialog',
						action: 'update',
						id: 'ESignatureDialog',
						control: providerLbJSON,
					},
					callback: this.callback.bind(this) as JSDialogCallback,
				});
			}
		}

		open(): void {
			const dialogBuildEvent = {
				data: this.getJSON(),
				callback: this.callback.bind(this) as JSDialogCallback,
			};
			app.map.fire('jsdialog', dialogBuildEvent);
		}

		setDefaultCountryCode(countryCode: string): void {
			this.defaultCountryCode = countryCode;
		}

		setDefaultProviderId(providerId: string): void {
			this.defaultProviderId = providerId;
		}
	}
}

JSDialog.eSignatureDialog = (
	countries: Array<cool.Country>,
	providers: Array<cool.SignatureProvider>,
) => {
	return new cool.ESignatureDialog(countries, providers);
};
