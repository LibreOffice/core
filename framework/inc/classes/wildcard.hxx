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

#ifndef INCLUDED_FRAMEWORK_INC_CLASSES_WILDCARD_HXX
#define INCLUDED_FRAMEWORK_INC_CLASSES_WILDCARD_HXX

#include <rtl/ustring.hxx>

namespace framework{

/*-************************************************************************************************************
    @short          implement wildcard-mechanism for unicode
    @descr          This class can be used to get information about the matching of a pattern to a given text.
                    It's suitable for 8-Bit- AND 16-Bit-strings!
    @ATTENTION      This class isn't threadsafe!

    @devstatus      deprecated
*//*-*************************************************************************************************************/

class Wildcard
{

    //  public methods

    public:

        //  constructor / destructor

        /*-****************************************************************************************************
            @short      standard ctor
            @descr      We do nothing here.
        *//*-*****************************************************************************************************/

        Wildcard();

        /*-****************************************************************************************************
            @short      standard dtor
            @descr      We do nothing here.
        *//*-*****************************************************************************************************/

        virtual ~Wildcard();

        //  interface

        /*-****************************************************************************************************
            @short      try to find an agreement between given text and searchpattern
            @descr      You can use wildcards in pattern only!
            @param      "sText" is the text, in which we search given pattern.
            @param      "sPattern" is the searched pattern with includes wildcards.
            @return     true , if pattern was found.
            @return     false, if pattern don't match the text.
        *//*-*****************************************************************************************************/

        static bool match(  const   OUString&    sText       ,
                            const   OUString&    sPattern    );

};      //  class Wildcard

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_CLASSES_WILDCARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
