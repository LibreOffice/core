/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stlsheet.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 13:13:15 $
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

#ifndef SC_STLSHEET_HXX
#define SC_STLSHEET_HXX

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

//------------------------------------------------------------------------

#define SCSTYLEBIT_STANDARD     0x0001

//------------------------------------------------------------------------

class ScStyleSheetPool;

class ScStyleSheet : public SfxStyleSheet
{
friend class ScStyleSheetPool;

public:

    enum    Usage
    {
        UNKNOWN,
        USED,
        NOTUSED
    };

private:
    mutable ScStyleSheet::Usage eUsage;

public:
                        TYPEINFO();

                        ScStyleSheet( const ScStyleSheet& rStyle );

    virtual BOOL        SetParent        ( const String& rParentName );
    virtual SfxItemSet& GetItemSet       ();
    virtual BOOL        IsUsed           () const;
    virtual BOOL        HasFollowSupport () const;
    virtual BOOL        HasParentSupport () const;

    virtual const String& GetName() const;
    virtual const String& GetParent() const;
    virtual const String& GetFollow() const;

    virtual BOOL SetName( const String& );

            void                SetUsage( ScStyleSheet::Usage eUse ) const
                                    { eUsage = eUse; }
            ScStyleSheet::Usage GetUsage() const
                                    { return eUsage; }

protected:
    virtual             ~ScStyleSheet();

                ScStyleSheet( const String&     rName,
                              ScStyleSheetPool& rPool,
                              SfxStyleFamily    eFamily,
                              USHORT            nMask );

    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                         const SfxHint& rHint, const TypeId& rHintType );
};

#endif     // SC_STLSHEET_HXX

