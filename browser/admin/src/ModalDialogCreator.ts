/* eslint-disable */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/// Available types: info, warning, danger, link, success, primary. Works with bulma.css.

// Every "set" function returns the instance. So you can do this:
// (new DlgYesNo).Title('some title').Text('some text').YesButtonText('yes').NoButtonText('no').YesFunction(function () {/* */}).NoFunction(function() {/** */});
// "Yes" and "No" buttons call callback function, close the modal and destroy the modal.
class DlgLoading {
    _instance: DlgLoading;
    _modalID: string = "loadingdlg"

    constructor() {
        this._instance = this;
        this.initialize();
    }

    private initialize() {
        let html: string = this.getModalHTML();
        let element: HTMLDivElement = document.createElement('div');
        element.innerHTML = html;
        document.getElementsByTagName('body')[0].appendChild(element);
    }

    private getModalHTML(): string {
        let html: string = ' \
        <div class="modal" id="modal-__created_id__"> \
            <div class="modal-background" id="modal-background-__created_id__"></div> \
            <div class="modal-card"> \
                <section class="modal-card-body" id="modal-body-__created_id__"> \
                </section> \
            </div> \
        </div>';

        html = html.split('__created_id__').join(String(this._modalID));
        return html;
    }

    text(text: string): DlgLoading {
        let body: HTMLDivElement = <HTMLDivElement>document.getElementById('modal-body-' + String(this._modalID));
        body.style.display = 'flex';
        body.style.alignItems = 'center';
        body.style.justifyContent = 'center';
        body.style.flexDirection = 'column';
        let progress: HTMLSpanElement = <HTMLSpanElement>document.createElement('progress');
        progress.className = 'is-info';

        let p: HTMLParagraphElement = <HTMLParagraphElement>document.createElement('p');
        p.textContent = text;
        body.appendChild(progress);
        body.appendChild(p);
        return this._instance;
    }

    open() {
        let modal = document.getElementById('modal-' + String(this._modalID));
        modal?.classList.add('is-active');
    }

    static close() {
        let modal = document.getElementById('modal-loadingdlg');
        modal?.classList.remove('is-active');
        modal?.parentNode?.removeChild(modal);
    }
}

class DlgYesNo {
    static _instanceCount: number = 0;
    _instance: DlgYesNo;
    _modalID: number;

    constructor() {
        this._instance = this;
        DlgYesNo._instanceCount++;
        this._modalID = DlgYesNo._instanceCount;
        this.initialize();
    }

    private initialize() {
        let html: string = this.getModalHTML();
        let element: HTMLDivElement = document.createElement('div');
        element.innerHTML = html;
        document.getElementsByTagName('body')[0].appendChild(element);
        this.initializeBackgroundClick();
        this.initializeCrossButton();
        this.initializeYesButton();
        this.initializeNoButton();
    }

    private initializeCrossButton() {
        let modal = document.getElementById('modal-' + String(this._modalID));
        let crossButton = document.getElementById('modal-cross-button-' + String(this._modalID))!;

        crossButton.onclick = function () {
            modal?.classList.remove('is-active');
            modal?.parentNode?.removeChild(modal);
        };
    }

    private initializeBackgroundClick() {
        let modal = document.getElementById('modal-' + String(this._modalID));
        let modalBackground = document.getElementById('modal-background-' + String(this._modalID))!;
        modalBackground.onclick = function () {
            modal?.classList.remove('is-active');
            modal?.parentNode?.removeChild(modal);
        };
    }

    private initializeYesButton() {
        let modal = document.getElementById('modal-' + String(this._modalID));
        let yesButton = document.getElementById('modal-yes-button-' + String(this._modalID))!;
        yesButton.onclick = function () {
            modal?.classList.remove('is-active');
            modal?.parentNode?.removeChild(modal);
        };
    }

    private initializeNoButton() {
        let modal = document.getElementById('modal-' + String(this._modalID));
        let noButton = document.getElementById('modal-no-button-' + String(this._modalID))!;
        noButton.onclick = function () {
            modal?.classList.remove('is-active');
            modal?.parentNode?.removeChild(modal);
        };
    }

    private getModalHTML(): string {
        let html: string = ' \
<div class="modal" id="modal-__created_id__"> \
    <div class="modal-background" id="modal-background-__created_id__"></div> \
    <div class="modal-card"> \
        <header class="modal-card-head" id="modal-head-__created_id__"> \
            <p class="modal-card-title" id="modal-title-__created_id__">Yes / No Modal Template</p> \
            <button class="delete" id="modal-cross-button-__created_id__"></button> \
        </header> \
        <section class="modal-card-body" id="modal-body-__created_id__">Yes / No Modal Body</section> \
        <footer class="modal-card-foot is-fullwidth" id="modal-foot-__created_id__"> \
            <button type="button" class="button is-pulled-left" id="modal-no-button-__created_id__" style="min-width:120px;">Cancel</button> \
            <button type="button" class="button is-pulled-right" id="modal-yes-button-__created_id__" style="min-width:120px;">OK</button> \
        </footer> \
    </div> \
</div>';

        html = html.split('__created_id__').join(String(this._modalID));
        return html;
    }

    yesButtonText(text: string): DlgYesNo {
        let button: HTMLButtonElement = <HTMLButtonElement>document.getElementById('modal-yes-button-' + String(this._modalID));
        button.innerText = text;
        return this._instance;
    }

    noButtonText(text: string): DlgYesNo {
        let button: HTMLButtonElement = <HTMLButtonElement>document.getElementById('modal-no-button-' + String(this._modalID));
        button.innerText = text;
        return this._instance;
    }

    title(text: string): DlgYesNo {
        let p: HTMLParagraphElement = <HTMLParagraphElement>document.getElementById('modal-title-' + String(this._modalID));
        p.innerText = text;
        return this._instance;
    }

    text(text: string): DlgYesNo {
        let d: HTMLDivElement = <HTMLDivElement>document.getElementById('modal-body-' + String(this._modalID));
        d.innerText = text;
        return this._instance;
    }

    type(type: string): DlgYesNo {
        let header: HTMLDivElement = <HTMLDivElement>document.getElementById('modal-head-' + String(this._modalID));
        header.className = 'modal-card-head has-background-' + type;
        return this._instance;
    }

    yesFunction(f: any): DlgYesNo {
        let modal = document.getElementById('modal-' + String(this._modalID));
        let yesButton = document.getElementById('modal-yes-button-' + String(this._modalID))!;
        yesButton.onclick = function (e: MouseEvent) {
            f(e);
            modal?.classList.remove('is-active');
            modal?.parentNode?.removeChild(modal);
        };
        return this._instance;
    }

    noFunction(f: any): DlgYesNo {
        let modal = document.getElementById('modal-' + String(this._modalID));
        let noButton = document.getElementById('modal-no-button-' + String(this._modalID))!;

        noButton.onclick = function (e: MouseEvent) {
            f(e);
            modal?.classList.remove('is-active');
            modal?.parentNode?.removeChild(modal);
        };
        return this._instance;
    }

    open() {
        let modal = document.getElementById('modal-' + String(this._modalID));
        modal?.classList.add('is-active');
    }
}
