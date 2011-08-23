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

#ifndef ARY_LOC_FILEBASE_HXX
#define ARY_LOC_FILEBASE_HXX

// USED SERVICES
    // BASE CLASSES
#include <ary/loc/loc_le.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace loc
{



/** Base class for classes representing source code files.
*/
class FileBase : public LocationEntity
{
  public:
    virtual			    ~FileBase() {}

  protected:
                        FileBase(
                            const String  &     i_localName ,
                            Le_id               i_parentDirectory );
  private:
    // Interface LocationEntity:
    virtual const String &
                        inq_LocalName() const;
    virtual Le_id       inq_ParentDirectory() const;

    // DATA
    String              sLocalName;
    Le_id               nParentDirectory;
};




} // namespace loc
} // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
