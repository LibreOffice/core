/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef __FRAMEWORK_THREADHELP_INONCOPYABLE_H_
#define __FRAMEWORK_THREADHELP_INONCOPYABLE_H_

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr          Use this as one of your base classes to disable
                    all possiblities to copy or assign one object to another one!
                    We declare neccessary functions private to do so.
*//*-*************************************************************************************************************/
class INonCopyable
{
    public:

        INonCopyable() {}

    private:

        INonCopyable            ( const INonCopyable& rCopy );
        INonCopyable& operator= ( const INonCopyable& rCopy );

};      //  class INonCopyable

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_INONCOPYABLE_H_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
