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

#ifndef CSV_X_HXX
#define CSV_X_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <cosv/string.hxx>
    // PARAMETERS
#include <cosv/csv_ostream.hxx>


namespace csv
{

class Exception
{
  public:
    virtual             ~Exception() {}
    virtual void        GetInfo(
                            ostream &           o_rOutputMedium ) const = 0;
};


class X_Default : public Exception
{
  public:
                        X_Default(
                            const char *        i_sMessage )
                            :   sMessage(i_sMessage) {}
    virtual void        GetInfo(                // Implemented in comfunc.cxx
                            ostream &           o_rOutputMedium ) const;
  private:
    String              sMessage;
};


}   // namespace csv



#endif



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
