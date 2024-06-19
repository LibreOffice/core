'use strict';

if (!('preRun' in Module)) Module['preRun'] = [];
Module.preRun.push(function() {
    ENV.SAL_LOG = "+WARN"
});
