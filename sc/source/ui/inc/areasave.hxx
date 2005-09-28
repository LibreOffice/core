/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: areasave.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-28 12:11:29 $
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

#ifndef SC_AREASAVE_HXX
#define SC_AREASAVE_HXX

#ifndef SC_COLLECT_HXX
#include "collect.hxx"
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

class ScDocument;
class ScAreaLink;


class ScAreaLinkSaver : public DataObject
{
private:
    String      aFileName;
    String      aFilterName;
    String      aOptions;
    String      aSourceArea;
    ScRange     aDestArea;
    ULONG       nRefresh;

public:
                ScAreaLinkSaver( const ScAreaLink& rSource );
                ScAreaLinkSaver( const ScAreaLinkSaver& rCopy );
    virtual     ~ScAreaLinkSaver();

    virtual DataObject* Clone() const;

    BOOL        IsEqual( const ScAreaLink& rCompare ) const;
    BOOL        IsEqualSource( const ScAreaLink& rCompare ) const;

    void        WriteToLink( ScAreaLink& rLink ) const;
    void        InsertNewLink( ScDocument* pDoc ) const;
};


class ScAreaLinkSaveCollection : public Collection
{
public:
                ScAreaLinkSaveCollection();
                ScAreaLinkSaveCollection( const ScAreaLinkSaveCollection& rCopy );
    virtual     ~ScAreaLinkSaveCollection();

    virtual DataObject* Clone() const;

    ScAreaLinkSaver*    operator[](USHORT nIndex) const {return (ScAreaLinkSaver*)At(nIndex);}

    BOOL        IsEqual( const ScDocument* pDoc ) const;
    void        Restore( ScDocument* pDoc ) const;

    // returns NULL if empty
    static ScAreaLinkSaveCollection* CreateFromDoc( const ScDocument* pDoc );
};


#endif

