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

#ifndef SC_DPOBJECT_HXX
#define SC_DPOBJECT_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_COLLECT_HXX
#include "collect.hxx"
#endif

#ifndef _COM_SUN_STAR_SHEET_XDIMENSIONSSUPPLIER_HPP_
#include <com/sun/star/sheet/XDimensionsSupplier.hpp>
#endif
class Rectangle;
class SvStream;
namespace binfilter {


//------------------------------------------------------------------


class ScDPSaveData;
class ScDPOutput;
struct ScDPPositionData;
class ScMultipleReadHeader;
class ScMultipleWriteHeader;
class ScPivot;
class ScPivotCollection;
struct ScPivotParam;
struct ScImportSourceDesc;
struct ScSheetSourceDesc;


struct ScDPServiceDesc
{
    String	aServiceName;
    String	aParSource;
    String	aParName;
    String	aParUser;
    String	aParPass;

    ScDPServiceDesc( const String& rServ, const String& rSrc, const String& rNam,
                        const String& rUser, const String& rPass ) :
        aServiceName( rServ ), aParSource( rSrc ), aParName( rNam ),
        aParUser( rUser ), aParPass( rPass ) {	}

    BOOL operator==	( const ScDPServiceDesc& rOther ) const
        { return aServiceName == rOther.aServiceName &&
                 aParSource   == rOther.aParSource &&
                 aParName     == rOther.aParName &&
                 aParUser     == rOther.aParUser &&
                 aParPass     == rOther.aParPass; }
};


class ScDPObject : public DataObject
{
private:
    ScDocument*				pDoc;
                                            // settings
    ScDPSaveData*			pSaveData;
    String					aTableName;
    String					aTableTag;
    ScRange					aOutRange;
    ScSheetSourceDesc*		pSheetDesc;		//	for sheet data
    ScImportSourceDesc* 	pImpDesc;		//	for database data
    ScDPServiceDesc*		pServDesc;		//	for external service
                                            // cached data
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDimensionsSupplier> xSource;
    ScDPOutput*				pOutput;
    BOOL					bSettingsChanged;
    BOOL					bAlive;			// FALSE if only used to hold settings


    void				CreateObjects();

public:
                ScDPObject( ScDocument* pD );
                ScDPObject(const ScDPObject& r);
    virtual		~ScDPObject();

    virtual	DataObject*	Clone() const;

    void				SetAlive(BOOL bSet);

    void				InvalidateData();
    void				InvalidateSource();


    void				SetSaveData(const ScDPSaveData& rData);
    ScDPSaveData*		GetSaveData() const		{ return pSaveData; }

    void				SetOutRange(const ScRange& rRange);
    const ScRange&		GetOutRange() const		{ return aOutRange; }

    void				SetSheetDesc(const ScSheetSourceDesc& rDesc);
    void				SetImportDesc(const ScImportSourceDesc& rDesc);
    void				SetServiceData(const ScDPServiceDesc& rDesc);


    const ScSheetSourceDesc* GetSheetDesc() const	{ return pSheetDesc; }
    const ScImportSourceDesc* GetImportSourceDesc() const	{ return pImpDesc; }
    const ScDPServiceDesc* GetDPServiceDesc() const	{ return pServDesc; }


    BOOL				IsSheetData() const;
    BOOL				IsImportData() const { return(pImpDesc != NULL); }
    BOOL				IsServiceData() const { return(pServDesc != NULL); }

    void				SetName(const String& rNew);
    const String&		GetName() const					{ return aTableName; }
    void				SetTag(const String& rNew);
    const String&		GetTag() const					{ return aTableTag; }



    BOOL				StoreOld(SvStream& rStream, ScMultipleWriteHeader& rHdr ) const;
    BOOL				StoreNew(SvStream& rStream, ScMultipleWriteHeader& rHdr ) const;
    BOOL				LoadNew(SvStream& rStream, ScMultipleReadHeader& rHdr );
    BOOL				FillOldParam(ScPivotParam& rParam, BOOL bForFile) const;
    void				InitFromOldPivot(const ScPivot& rOld, ScDocument* pDoc, BOOL bSetSource);

    void				UpdateReference( UpdateRefMode eUpdateRefMode,
                                          const ScRange& r, short nDx, short nDy, short nDz );

    static void			ConvertOrientation( ScDPSaveData& rSaveData,
                            PivotField* pFields, USHORT nCount, USHORT nOrient,
                            ScDocument* pDoc, USHORT nRow, USHORT nTab,
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XDimensionsSupplier>& xSource,
                            BOOL bOldDefaults,
                            PivotField* pRefColFields = NULL, USHORT nRefColCount = 0,
                            PivotField* pRefRowFields = NULL, USHORT nRefRowCount = 0 );
};


class ScDPCollection : public Collection
{
private:
    ScDocument*	pDoc;

public:
                ScDPCollection(ScDocument* pDocument);
                ScDPCollection(const ScDPCollection& r);
    virtual		~ScDPCollection();

    virtual	DataObject*	Clone() const;

    ScDPObject*	operator[](USHORT nIndex) const {return (ScDPObject*)At(nIndex);}

    BOOL		StoreOld( SvStream& rStream ) const;
    BOOL		StoreNew( SvStream& rStream ) const;
    BOOL		LoadNew( SvStream& rStream );

    void		ConvertOldTables( ScPivotCollection& rOldColl );

    void		UpdateReference( UpdateRefMode eUpdateRefMode,
                                 const ScRange& r, short nDx, short nDy, short nDz );

    String 		CreateNewName( USHORT nMin = 1 ) const;
    void		EnsureNames();
};


} //namespace binfilter
#endif

