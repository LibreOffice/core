# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
""" Bootstrap PyUNO Runtime.
The soffice process is started opening a named pipe of random name, then
the local context is used to access the pipe. This function directly
returns the remote component context, from whereon you can get the
ServiceManager by calling getServiceManager() on the returned object.
This module supports the following environments:
-   Windows
-   GNU/Linux derivatives
-   Mac OS X
A configurable time-out allows to wait for LibO process to be completed.
Multiple attempts can be set in order to connect to LibO as a service.
Specific versions of the office suite can be started.
Instructions:
1.  Include one of the below examples in your Python macro
2.  Run your LibreOffice script from your preferred IDE
Imports:
    os, random, subprocess, sys - `bootstrap`
    itertools, time - `retry` decorator
Exceptions:
    OSError            - in `bootstrap`
    BootstrapException - in `bootstrap`
    NoConnectException - in `bootstrap`
Functions:
    `bootstrap`
    `retry` decorator
Acknowledgments:
  - Kim Kulak for original officehelper.py Python translation from bootstrap.java
  - ActiveState, for `retry` python decorator
warning:: Tested platforms are Linux, Mac OS X & Windows
    AppImage, Flatpak, Snap and the like have not been validated
:References:
.. _ActiveState retry Python decorator: http://code.activestate.com/recipes/580745-retry-decorator-in-python/
"""
import os, random, signal, subprocess  # in bootstrap()
from sys import platform  # in bootstrap()
import itertools, time  # in retry() decorator
import uno
from com.sun.star.connection import NoConnectException
from com.sun.star.uno import Exception as UnoException


class BootstrapException(UnoException):
    pass


def retry(delays=(0, 1, 5, 30, 180, 600, 3600),
          exception=Exception,
          report=lambda *args: None):
    """retry Python decorator
    Credit:
    http://code.activestate.com/recipes/580745-retry-decorator-in-python/
    """
    def wrapper(function):
        def wrapped(*args, **kwargs):
            problems = []
            for delay in itertools.chain(delays, [None]):
                try:
                    return function(*args, **kwargs)
                except exception as problem:
                    problems.append(problem)
                    if delay is None:
                        report("retryable failed definitely:", problems)
                        raise
                    else:
                        report("retryable failed:", problem,
                               "-- delaying for %ds" % delay)
                        time.sleep(delay)
            return None
        return wrapped
    return wrapper


def bootstrap(soffice=None, delays=(1, 3, 5, 7), report=lambda *args: None):
    # 4 connection attempts; sleeping 1, 3, 5 and 7 seconds
    # no report to console
    """Bootstrap PyUNO Runtime.
    The soffice process is started opening a named pipe of random name,
    then the local context is used to access the pipe. This function
    directly returns the remote component context, from whereon you can
    get the ServiceManager by calling getServiceManager() on the
    returned object.

    Examples:
    i.  Start LO as a service, get its remote component context

        import officehelper
        ctx = officehelper.bootstrap()
        # your code goes here

    ii. Wait longer for LO to start, request context multiples times
      + Report processing in console

        import officehelper as oh
        ctx = oh.bootstrap(delays=(5,10,15,20),report=print)  # wait 5, 10, 15 and 20 sec.
        # your code goes here

    iii. Use a specific LibreOffice copy

        from officehelper import bootstrap
        ctx = bootstrap(soffice=r"USB:\PortableApps\libO-7.6\App\libreoffice\program\soffice.exe")
        # your code goes here
    """
    try:
        process = None  # used in except clause
        if soffice:  # soffice fully qualified path as parm
            sOffice = soffice
        else:  # soffice script used on *ix, Mac; soffice.exe used on Win
            if "UNO_PATH" in os.environ:
                sOffice = os.environ["UNO_PATH"]
            else:
                sOffice = ""  # let's hope for the best
            sOffice = os.path.join(sOffice, "soffice")
            if platform.startswith("win"):
                sOffice += ".exe"
                sOffice = '"' + sOffice + '"'  # accommodate ' ' spaces in filename
            elif platform == "darwin":  # any other un-hardcoded suggestion?
                sOffice = "/Applications/LibreOffice.App/Contents/MacOS/soffice"
        # Generate a random pipe name.
        random.seed()
        sPipeName = "uno" + str(random.random())[2:]
        # Start the office process
        connect_string = ''.join(['pipe,name=', sPipeName, ';urp;'])
        command = ' '.join([sOffice, '--nologo', '--nodefault', '--accept="' + connect_string + '"'])
        if platform.startswith("win") or platform == "darwin":
            process = subprocess.Popen(command, shell=True)
        elif platform == "linux":  # Use a process group to enable proper termination
            process = subprocess.Popen(command, shell=True, preexec_fn=os.setsid)
        else:
            raise OSError
        # Connect to a started office instance
        xLocalContext = uno.getComponentContext()
        resolver = xLocalContext.ServiceManager.createInstanceWithContext(
            "com.sun.star.bridge.UnoUrlResolver", xLocalContext)
        sConnect = "".join(['uno:', connect_string, 'StarOffice.ComponentContext'])
        @retry(delays=delays,
               exception=NoConnectException,
               report=report)
        def resolve():
            return resolver.resolve(sConnect)  # may raise NoConnectException
        ctx = resolve()
        return ctx

    except Exception:
        time.sleep(10)
        if process:  # clean memory from soffice running process
            if platform.startswith("win") or platform == "darwin":
                process.terminate()  # Send termination signal
            elif platform == "linux":  
                os.killpg(os.getpgid(process.pid), signal.SIGTERM)  # Send termination signal to process group
        raise BootstrapException

# vim: set shiftwidth=4 softtabstop=4 expandtab
