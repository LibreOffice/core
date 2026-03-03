Module.preRun = Module.preRun || [];
Module.preRun.push((Module) => Module.FS.mkdir('/tempdoc'));
