/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: misccfg.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _SFX_MISCCFG_HXX
#define _SFX_MISCCFG_HXX

#include "svtools/svldllapi.h"
#include "tools/solar.h"
#include "unotools/configitem.hxx"

/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/

class SVL_DLLPUBLIC SfxMiscCfg : public utl::ConfigItem
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


