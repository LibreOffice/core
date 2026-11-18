/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/*
 * Utility class
 */

/* global Base _ */

/* eslint no-unused-vars:0 */

var Util = Base.extend({
	constructor: null

}, { // class interface

	humanizeMem: function (kbytes) {
		var unit = 1024;
		var units = [_('kB'), _('MB'), _('GB'), _('TB'), _('PB'), _('EB'), _('ZB'), _('YB'), _('BB')];
		for (var i = 0; Math.abs(kbytes) >= unit && i < units.length; i++) {
			kbytes /= unit;
		}

		return kbytes.toFixed(1) + ' ' + units[i];
	},

    /// Return human readable quantity with added multiple, percentage (1/100) or permyriad (1/10'000) to maximum, if maximum > 1.
    humanizeQty: function (quantity, maximum) {
        var qtyPrecision = 1;
        var pct_s = '';
        if (maximum > 1) {
            qtyPrecision = 0;
            var pct = ( 100 * quantity ) / maximum;
            if( pct > 100 ) {
                pct = quantity / maximum;
                pct_s = ', ' + pct.toFixed(1) + 'x';
            } else if( pct >= 10 ) {
                pct_s = ', ' + pct.toFixed(0) + '%';
            } else if( pct >= 0.1 ) {
                pct_s = ', ' + pct.toFixed(1) + '%';
            } else {
                pct = ( 10000 * quantity ) / maximum;
                if( pct >= 10 ) {
                    pct_s = ', ' + pct.toFixed(0) + '‱';
                } else {
                    pct_s = ', ' + pct.toFixed(1) + '‱';
                }
            }
        }
        var unit = 1000;
        var units = ['', 'k', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y', 'B'];
        for (var i = 0; Math.abs(quantity) >= unit && i < units.length; i++) {
            quantity /= unit;
        }
        return quantity.toFixed(qtyPrecision) + units[i] + pct_s;
    },

	humanizeSecs: function(secs) {
		var mins = 0;
		var hrs = 0;
		var res = '';

		secs = parseInt(secs);
		if (isNaN(secs)) {
			return res;
		}

		if (secs >= 60) {
			mins = Math.floor(secs / 60);
			secs = secs - mins * 60;
		}
		if (mins >= 60) {
			hrs = Math.floor(mins / 60);
			mins = mins - hrs * 60;
		}

		if (hrs) {
			if (mins < 10) {
				res = hrs + ':0' + mins + _(' hrs');
			} else {
				res = hrs + ':' + mins + _(' hrs');
			}
		} else if (mins) {
			if (secs < 10) {
				res = mins + ':0' + secs + _(' mins');
			} else {
				res = mins + ':' + secs + _(' mins');
			}
		} else if (secs) {
			res = secs + _(' s');
		} else {
			res = '';
		}

		return res;
	},

	consumeDataText: function(textInput, dataArray) {
		textInput = textInput.split(' ')[1];
		if (textInput.endsWith(',')) {
			// This is the result of query, not notification
			var i, j, data;
			data = textInput.substring(0, textInput.length - 1).split(',');
			for (i = dataArray.length - 1, j = data.length - 1; i >= 0 && j >= 0; i--, j--) {
				dataArray[i].value = parseInt(data[j]);
			}
			return undefined;
		}
		else {
			// this is a notification data; append to dataArray
			return textInput.trim();
		}
	}
});
