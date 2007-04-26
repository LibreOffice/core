/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svtdata.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-26 09:41:48 $
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

#ifndef _SVTOOLS_SVTDATA_HXX
#define _SVTOOLS_SVTDATA_HXX

#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _TOOLS_SIMPLERESMGR_HXX_
#include <tools/simplerm.hxx>
#endif

class ResMgr;
class SfxItemDesruptorList_Impl;
class SfxItemPool;
class Twain;

//============================================================================
class ImpSvtData
{
public:
    Twain * pTwain;
    const SfxItemPool * pStoringPool;
    SfxItemDesruptorList_Impl * pItemDesruptList;

    ResMgr *        pResMgr;
    ResMgr *        pPatchResMgr;

    void*           m_pThreadsafeRMs;
        // one SimpleResMgr for each language for which a resource was requested
        // (When using the 'non-simple' resmgr, the first request for any language wins, any
        // further request for any other language supply the resmgr of the first call.
        // For the simple resmgr we have a mgr for each language ever requested).

private:
    ImpSvtData():
        pTwain(0), pStoringPool(0), pItemDesruptList(0), pResMgr(0),
        pPatchResMgr(NULL), m_pThreadsafeRMs(NULL)
    {}

    ~ImpSvtData();

public:
    ResMgr * GetResMgr(const ::com::sun::star::lang::Locale aLocale);
    ResMgr * GetResMgr(); // VCL dependant, only available in SVT, not in SVL!

    ResMgr * GetPatchResMgr();
    ResMgr * GetPatchResMgr(const ::com::sun::star::lang::Locale& aLocale);


    SimpleResMgr * GetSimpleRM(const ::com::sun::star::lang::Locale& rLocale);

    static ImpSvtData & GetSvtData();
};

//============================================================================

class SvpResId: public ResId
{
public:
    SvpResId( USHORT nId, const ::com::sun::star::lang::Locale aLocale ):
        ResId( nId, *ImpSvtData::GetSvtData().GetResMgr( aLocale ) ) {}

     // VCL dependant, only available in SVT, not in SVL!
    SvpResId( USHORT nId );
};


class SvtResId: public ResId
{
public:
    SvtResId(USHORT nId, const ::com::sun::star::lang::Locale aLocale):
        ResId(nId, *ImpSvtData::GetSvtData().GetResMgr(aLocale)) {}

    SvtResId(USHORT nId): ResId(nId, *ImpSvtData::GetSvtData().GetResMgr()) {}
     // VCL dependant, only available in SVT, not in SVL!
};

//============================================================================
class SvtSimpleResId
{
    String  m_sValue;

public:
    SvtSimpleResId(USHORT nId, const ::com::sun::star::lang::Locale aLocale) : m_sValue(ImpSvtData::GetSvtData().GetSimpleRM(aLocale)->ReadString(nId)) { };

    operator String () const { return m_sValue; }
};



#endif //  _SVTOOLS_SVTDATA_HXX

