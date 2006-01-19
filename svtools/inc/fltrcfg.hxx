/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fltrcfg.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-01-19 18:33:36 $
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
#ifndef _SVT_FLTRCFG_HXX
#define _SVT_FLTRCFG_HXX

// -----------------------------------------------------------------------

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

struct SvtFilterOptions_Impl;
class SVL_DLLPUBLIC SvtFilterOptions : public utl::ConfigItem
{
    SvtFilterOptions_Impl* pImp;

    const com::sun::star::uno::Sequence<rtl::OUString>& GetPropertyNames();
public:
                    SvtFilterOptions();
    virtual        ~SvtFilterOptions();

    virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
    virtual void            Commit();
    void                    Load();

    void SetLoadWordBasicCode( sal_Bool bFlag );
    sal_Bool IsLoadWordBasicCode() const;
    void SetLoadWordBasicStorage( sal_Bool bFlag );
    sal_Bool IsLoadWordBasicStorage() const;

    void SetLoadExcelBasicCode( sal_Bool bFlag );
    sal_Bool IsLoadExcelBasicCode() const;
    void SetLoadExcelBasicStorage( sal_Bool bFlag );
    sal_Bool IsLoadExcelBasicStorage() const;

    void SetLoadPPointBasicCode( sal_Bool bFlag );
    sal_Bool IsLoadPPointBasicCode() const;
    void SetLoadPPointBasicStorage( sal_Bool bFlag );
    sal_Bool IsLoadPPointBasicStorage() const;

    sal_Bool IsMathType2Math() const;
    void SetMathType2Math( sal_Bool bFlag );
    sal_Bool IsMath2MathType() const;
    void SetMath2MathType( sal_Bool bFlag );

    sal_Bool IsWinWord2Writer() const;
    void SetWinWord2Writer( sal_Bool bFlag );
    sal_Bool IsWriter2WinWord() const;
    void SetWriter2WinWord( sal_Bool bFlag );

    sal_Bool IsExcel2Calc() const;
    void SetExcel2Calc( sal_Bool bFlag );
    sal_Bool IsCalc2Excel() const;
    void SetCalc2Excel( sal_Bool bFlag );

    sal_Bool IsPowerPoint2Impress() const;
    void SetPowerPoint2Impress( sal_Bool bFlag );
    sal_Bool IsImpress2PowerPoint() const;
    void SetImpress2PowerPoint( sal_Bool bFlag );

    static SvtFilterOptions* Get();
};

#endif




