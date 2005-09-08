/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: misccfg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:57:41 $
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
#ifndef _SFX_MISCCFG_HXX
#define _SFX_MISCCFG_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SOLAR_H
#include "tools/solar.h"
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include "unotools/configitem.hxx"
#endif

/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/

class SVT_DLLPUBLIC SfxMiscCfg : public utl::ConfigItem
{
    BOOL            bPaperSize;     // printer warnings
    BOOL            bPaperOrientation;
    BOOL            bNotFound;
    sal_Int32       nYear2000;      // two digit year representation

    const com::sun::star::uno::Sequence<rtl::OUString>& GetPropertyNames();
    void                    Load();

public:
    SfxMiscCfg( );
    ~SfxMiscCfg( );

    virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
    virtual void            Commit();

    BOOL        IsNotFoundWarning()     const {return bNotFound;}
    void        SetNotFoundWarning( BOOL bSet);

    BOOL        IsPaperSizeWarning()    const {return bPaperSize;}
    void        SetPaperSizeWarning(BOOL bSet);

    BOOL        IsPaperOrientationWarning()     const {return bPaperOrientation;}
    void        SetPaperOrientationWarning( BOOL bSet);

                // 0 ... 99
    sal_Int32   GetYear2000()           const { return nYear2000; }
    void        SetYear2000( sal_Int32 nSet );

};

#endif // _MISCCFG_HXX


