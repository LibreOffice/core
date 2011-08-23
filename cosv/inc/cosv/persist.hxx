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

#ifndef CSV_PERSIST_HXX
#define CSV_PERSIST_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <cosv/string.hxx>
    // PARAMETERS


namespace csv
{
namespace ploc
{

class Path;


inline const char *
Delimiter()
{
#ifdef WNT
    return "\\";
#elif defined(UNX)
    return "/";
#else
#error  For using csv::ploc there has to be defined: WNT or UNX.
#endif
}



class Persistent
{
  public:
    virtual             ~Persistent() {}

    const Path &        MyPath() const;
    /// @return all pathes without completing delimiter, even directories.
    const char *        StrPath() const;
    bool                Exists() const;

  protected:
                        Persistent();
    void                InvalidatePath();

  private:
    virtual const Path &
                        inq_MyPath() const = 0;
    // DATA
    mutable StreamStr   sPath;
};



// IMPLEMENTATION

inline
Persistent::Persistent()
    :   sPath(30) { }
inline const Path &
Persistent::MyPath() const
    { return inq_MyPath(); }
inline void
Persistent::InvalidatePath()
    { sPath.clear(); }



}   // namespace csv
}   // namespace ploc


#endif



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
