/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmtautofmt.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 15:32:02 $
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
#ifndef _FMTAUTOFMT_HXX
#define _FMTAUTOFMT_HXX


#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif
#include <boost/shared_ptr.hpp>

// ATT_AUTOFMT *********************************************


class SwFmtAutoFmt: public SfxPoolItem
{
    boost::shared_ptr<SfxItemSet> mpHandle;

public:
    SwFmtAutoFmt( USHORT nWhich = RES_TXTATR_AUTOFMT );

    // single argument ctors shall be explicit.
    virtual ~SwFmtAutoFmt();

    // @@@ public copy ctor, but no copy assignment?
    SwFmtAutoFmt( const SwFmtAutoFmt& rAttr );
private:
    // @@@ public copy ctor, but no copy assignment?
    SwFmtAutoFmt & operator= (const SwFmtAutoFmt &);
public:

    TYPEINFO();

    // "pure virtual methods" of SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    virtual BOOL             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual BOOL             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    void SetStyleHandle( boost::shared_ptr<SfxItemSet> pHandle ) { mpHandle = pHandle; }
    const boost::shared_ptr<SfxItemSet> GetStyleHandle() const { return mpHandle; }
          boost::shared_ptr<SfxItemSet> GetStyleHandle() { return mpHandle; }
};

#endif

