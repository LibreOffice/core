/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef CSV_PLOCROOT_HXX
#define CSV_PLOCROOT_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <cosv/string.hxx>
    // PARAMETERS
#include <cosv/csv_ostream.hxx>
#include <cosv/persist.hxx>


namespace csv
{

class bostream;


namespace ploc
{


class Root
{
  public:
    virtual             ~Root();

    static DYN Root *   Create_(
                            const char * &      o_sPathAfterRoot,
                            const char *        i_sPath,
                            const char *        i_sDelimiter = Delimiter() );

    virtual void        Get(                    /// Does not add a '\0' at the end,
                            ostream &           o_rPath ) const = 0;
    virtual void        Get(                    /// Does not add a '\0' at the end.
                            bostream &          so_rPath ) const = 0;
    virtual DYN Root *  CreateCopy() const = 0;
    virtual const char *
                        OwnDelimiter() const = 0;
};



}   // namespace ploc
}   // namespace csv



#endif



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
