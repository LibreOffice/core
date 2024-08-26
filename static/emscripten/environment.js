if (!('preRun' in Module)) Module['preRun'] = [];
Module.preRun.push(function() {
    ENV.MAX_CONCURRENCY = '4';
    ENV.SAL_LOG = "+WARN"
});
Module.ignoreApplicationExit = true;
