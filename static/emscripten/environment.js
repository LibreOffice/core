if (!('preRun' in Module)) Module['preRun'] = [];
Module.preRun.push(function() {
    ENV.SAL_LOG = "+WARN"
    ENV.SAL_VCL_QT5_USE_CAIRO = "1"
});
