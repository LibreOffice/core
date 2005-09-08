/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: persist.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:56:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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



