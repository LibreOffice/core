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
# the path to the office installation (e.g. /home/joe/OpenOffice.org1.1Beta)
setenv OOOHOME /src4/OpenOffice.org1.1Beta2

# don't modify anything beyond these lines
#---------------------------------------------
setenv PYTHONHOME $OOOHOME/program/python

if( ! $?LD_LIBRARY_PATH  ) then
    setenv LD_LIBRARY_PATH
endif

if(! $?PYTHONPATH ) then
    setenv PYTHONPATH
endif

if( ! $?LD_LIBRARY_PATH ) then
setenv LD_LIBRARY_PATH
endif

if( "$PYTHONPATH" != "" ) then
    setenv PYTHONPATH $OOOHOME/program:$OOOHOME/program/pydemo:$OOOHOME/program/python/lib:$PYTHONPATH
else
    setenv PYTHONPATH $OOOHOME/program:$OOOHOME/program/pydemo:$OOOHOME/program/python/lib
endif

setenv LD_LIBRARY_PATH $OOOHOME/program:$LD_LIBRARY_PATH

if( $?PYTHONHOME ) then
setenv PATH $PYTHONHOME/bin:$PATH
endif

# vim:set shiftwidth=4 softtabstop=4 expandtab:
