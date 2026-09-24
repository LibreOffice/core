/* -*- js-indent-level: 8 -*- */
/* global AdminSocketBase Admin $ */

var AdminClusterOverviewAbout = AdminSocketBase.extend({
    constructor: function(host, routeToken) {
        this.base(host);
        this.routeToken = routeToken;
    },

    onSocketMessage: function(e) {
        var textMsg;
        if (typeof e.data === 'string') {
            textMsg = e.data;
        }
        else {
            textMsg = '';
        }
        if (textMsg.startsWith('license')) {
            // eslint-disable-next-line no-restricted-syntax -- license page markup built into the controller
            $('#license-content').html(textMsg.substring('license: '.length));
        }
    },

    onSocketOpen: function() {
        this.socket.send('auth jwt=' + window.jwtToken + ' routeToken=' + this.routeToken);
        this.socket.send('license');
    },

    onSocketClose: function() {
        this.base.call(this);
    }
});

Admin.ClusterOverviewAbout = function(host, routeToken) {
    return new AdminClusterOverviewAbout(host, routeToken);
};
