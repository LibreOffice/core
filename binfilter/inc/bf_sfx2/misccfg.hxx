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
#ifndef _SFX_MISCCFG_HXX
#define _SFX_MISCCFG_HXX

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
namespace binfilter {

/*--------------------------------------------------------------------
     Beschreibung:
 --------------------------------------------------------------------*/

class SfxMiscCfg : public ::utl::ConfigItem
{
    BOOL			bPaperSize;     // printer warnings
    BOOL			bPaperOrientation;
    BOOL			bNotFound;
    sal_Int32		nYear2000;		// two digit year representation

    const ::com::sun::star::uno::Sequence<rtl::OUString>& GetPropertyNames();
    void					Load();

public:
    SfxMiscCfg( );
    ~SfxMiscCfg( );

    virtual void 			Notify( const ::com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
    virtual void			Commit();

    BOOL		IsNotFoundWarning() 	const {return bNotFound;}

    BOOL		IsPaperSizeWarning() 	const {return bPaperSize;}

    BOOL		IsPaperOrientationWarning() 	const {return bPaperOrientation;}
                // 0 ... 99
    sal_Int32	GetYear2000()			const { return nYear2000; }
};

}//end of namespace binfilter
#endif // _MISCCFG_HXX

