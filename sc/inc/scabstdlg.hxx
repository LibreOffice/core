/*************************************************************************
 *
 *  $RCSfile: scabstdlg.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 12:52:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SC_ABSTDLG_HXX
#define _SC_ABSTDLG_HXX

// include ---------------------------------------------------------------

#include <tools/solar.h>
#include <tools/string.hxx>
#include <sfx2/sfxdlg.hxx>
#include <vcl/syswin.hxx>
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#include "sc.hrc"
#include "global.hxx"
#include "pivot.hxx"

class ScAsciiOptions;
class ScAutoFormat;
class ScAutoFormatData;
class ScDocument;
struct ScImportSourceDesc;
class ScViewData;
class ScRangeName;
class ScQueryItem;
class ScImportOptions;
class SfxStyleSheetBase;
class ScDPObject;
struct ScDPFuncData;

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldReference;
} } } }

class AbstractScImportAsciiDlg : public VclAbstractDialog  //add for ScImportAsciiDlg
{
public:
    virtual void                        GetOptions( ScAsciiOptions& rOpt ) = 0;
};


class AbstractScAutoFormatDlg : public VclAbstractDialog  //add for ScAutoFormatDlg
{
public:
    virtual USHORT GetIndex() const = 0 ;
    virtual String GetCurrFormatName() = 0;
};

class AbstractScColRowLabelDlg : public VclAbstractDialog  //add for ScColRowLabelDlg
{
public:
    virtual BOOL IsCol() = 0;
    virtual BOOL IsRow() = 0;
};

class AbstractScDataPilotDatabaseDlg  :public VclAbstractDialog  //add for ScDataPilotDatabaseDlg
{
public:
    virtual void    GetValues( ScImportSourceDesc& rDesc ) = 0;
};

class AbstractScDataPilotSourceTypeDlg  : public VclAbstractDialog  //add for ScDataPilotSourceTypeDlg
{
public:
    virtual BOOL    IsDatabase() const = 0;
    virtual BOOL    IsExternal() const = 0;
};

class AbstractScDataPilotServiceDlg : public VclAbstractDialog  //add for ScDataPilotServiceDlg
{
public:
    virtual String  GetServiceName() const = 0;
    virtual String  GetParSource() const = 0 ;
    virtual String  GetParName() const = 0 ;
    virtual String  GetParUser() const = 0;
    virtual String  GetParPass() const = 0;
};

class AbstractScDeleteCellDlg : public VclAbstractDialog  //add for ScDeleteCellDlg
{
public:
    virtual DelCellCmd GetDelCellCmd() const = 0;
};

class AbstractScDeleteContentsDlg: public VclAbstractDialog  //add for ScDeleteContentsDlg
{
public:
    virtual void    DisableObjects() = 0 ;
    virtual USHORT  GetDelContentsCmdBits() const = 0;
};

class AbstractScFillSeriesDlg: public VclAbstractDialog  //add for ScFillSeriesDlg
{
public:
    virtual FillDir     GetFillDir() const = 0;
    virtual FillCmd     GetFillCmd() const = 0;
    virtual FillDateCmd GetFillDateCmd() const = 0;
    virtual double      GetStart() const = 0;
    virtual double      GetStep() const = 0;
    virtual double      GetMax() const = 0;
    virtual String      GetStartStr() const = 0;
    virtual void        SetEdStartValEnabled(BOOL bFlag=FALSE) = 0;
};

class AbstractScGroupDlg :  public VclAbstractDialog  //add for ScGroupDlg
{
public:
    virtual BOOL GetColsChecked() const = 0;
};

class AbstractScInsertCellDlg : public VclAbstractDialog  //add for ScInsertCellDlg
{
public:
    virtual InsCellCmd GetInsCellCmd() const = 0;
};

class AbstractScInsertContentsDlg : public VclAbstractDialog  //add for ScInsertContentsDlg
{
public:
    virtual USHORT      GetInsContentsCmdBits() const = 0;
    virtual USHORT      GetFormulaCmdBits() const = 0 ;
    virtual BOOL        IsSkipEmptyCells() const = 0;
    virtual BOOL        IsLink() const = 0;
    virtual void    SetFillMode( BOOL bSet ) = 0;
    virtual void    SetOtherDoc( BOOL bSet ) = 0;
    virtual BOOL        IsTranspose() const = 0;
    virtual void    SetChangeTrack( BOOL bSet ) = 0;
    virtual void    SetCellShiftDisabled( int nDisable ) = 0;
    virtual InsCellCmd  GetMoveMode() = 0;
};

class AbstractScInsertTableDlg : public VclAbstractDialog  //add for ScInsertTableDlg
{
public:
    virtual BOOL            GetTablesFromFile() = 0;
    virtual BOOL            GetTablesAsLink()   = 0;
    virtual const String*   GetFirstTable( USHORT* pN = NULL ) = 0;
    virtual ScDocShell*     GetDocShellTables() = 0;
    virtual BOOL            IsTableBefore() = 0;
    virtual USHORT          GetTableCount() = 0;
    virtual const String*   GetNextTable( USHORT* pN = NULL ) = 0;

};

class AbstractScSelEntryDlg : public VclAbstractDialog  //add for ScSelEntryDlg
{
public:
    virtual String GetSelectEntry() const = 0;
};

class AbstractScLinkedAreaDlg : public VclAbstractDialog  //add for ScLinkedAreaDlg
{
public:

    virtual void            InitFromOldLink( const String& rFile, const String& rFilter,
                                        const String& rOptions, const String& rSource,
                                        ULONG nRefresh ) = 0;
    virtual String          GetURL() = 0;
    virtual String          GetFilter() = 0;        // may be empty
    virtual String          GetOptions() = 0;       // filter options
    virtual String          GetSource() = 0;        // separated by ";"
    virtual ULONG           GetRefresh() = 0;       // 0 if disabled
};

class AbstractScMetricInputDlg : public VclAbstractDialog  //add for ScMetricInputDlg
{
public:
    virtual long GetInputValue( FieldUnit eUnit = FUNIT_TWIP ) const = 0;
};

class AbstractScMoveTableDlg : public VclAbstractDialog  //add for ScMoveTableDlg
{
public:
    virtual USHORT  GetSelectedDocument     () const = 0;
    virtual USHORT  GetSelectedTable        () const = 0;
    virtual BOOL    GetCopyTable            () const = 0;
    virtual void    SetCopyTable            (BOOL bFlag=TRUE) = 0;
    virtual void    EnableCopyTable         (BOOL bFlag=TRUE) = 0;
};

class AbstractScNameCreateDlg : public VclAbstractDialog  //add for ScNameCreateDlg
{
public:
    virtual USHORT          GetFlags() const = 0;
};

class AbstractScNamePasteDlg : public VclAbstractDialog  //add for ScNamePasteDlg
{
public:
    virtual String          GetSelectedName() const = 0;
};

class AbstractScPivotFilterDlg : public VclAbstractDialog  //add for ScPivotFilterDlg
{
public:
    virtual const ScQueryItem&  GetOutputItem() = 0;
};

class AbstractScDPFunctionDlg : public VclAbstractDialog  //add for ScDPFunctionDlg
{
public:
    virtual USHORT  GetFuncMask() const = 0;
    virtual ::com::sun::star::sheet::DataPilotFieldReference GetFieldRef() const = 0;
};

class AbstractScDPSubtotalDlg : public VclAbstractDialog  //add for ScDPSubtotalDlg
{
public:
    virtual USHORT  GetFuncMask() const = 0;
    virtual void    FillLabelData( LabelData& rLabelData ) const = 0;
};

class AbstractScDPShowDetailDlg : public VclAbstractDialog  //add for ScDPShowDetailDlg
{
public:
    virtual String  GetDimensionName() const = 0;
};

class AbstractScNewScenarioDlg : public VclAbstractDialog  //add for ScNewScenarioDlg
{
public:

    virtual void SetScenarioData( const String& rName, const String& rComment,
                            const Color& rColor, USHORT nFlags ) = 0;

    virtual void GetScenarioData( String& rName, String& rComment,
                            Color& rColor, USHORT& rFlags ) const = 0;
};

class AbstractScShowTabDlg : public VclAbstractDialog  //add for ScShowTabDlg
{
public:
    virtual void    Insert( const String& rString, BOOL bSelected ) = 0;
    virtual USHORT  GetSelectEntryCount() const = 0;
    virtual void SetDescription(const String& rTitle, const String& rFixedText,ULONG nDlgHelpId, ULONG nLbHelpId ) = 0;
    virtual String  GetSelectEntry(USHORT nPos) const = 0;
    virtual USHORT  GetSelectEntryPos(USHORT nPos) const = 0;
};

class AbstractScStringInputDlg :  public VclAbstractDialog  //add for ScStringInputDlg
{
public:
    virtual void GetInputString( String& rString ) const = 0;
};

class AbstractScImportOptionsDlg : public VclAbstractDialog  //add for ScImportOptionsDlg
{
public:
    virtual void GetImportOptions( ScImportOptions& rOptions ) const = 0;
};
//-------Scabstract fractory ---------------------------
class ScAbstractDialogFactory
{
public:
    static ScAbstractDialogFactory*     Create();

    virtual     AbstractScImportAsciiDlg * CreateScImportAsciiDlg( Window* pParent, String aDatName, //add for ScImportAsciiDlg
                                                                    SvStream* pInStream, const ResId& rResId,
                                                                    sal_Unicode cSep = '\t') = 0;

    virtual     AbstractScAutoFormatDlg * CreateScAutoFormatDlg( Window*                    pParent, //add for ScAutoFormatDlg
                                                                ScAutoFormat*               pAutoFormat,
                                                                const ScAutoFormatData*    pSelFormatData,
                                                                ScDocument*                pDoc,
                                                                const ResId& rResId) = 0;
    virtual AbstractScColRowLabelDlg * CreateScColRowLabelDlg (Window* pParent, //add for ScColRowLabelDlg
                                                                const ResId& rResId,
                                                                BOOL bCol = FALSE,
                                                                BOOL bRow = FALSE) = 0;

    virtual VclAbstractDialog * CreateScColOrRowDlg( Window*            pParent, //add for ScColOrRowDlg
                                                    const String&   rStrTitle,
                                                    const String&   rStrLabel,
                                                    const ResId& rResId,
                                                    BOOL                bColDefault = TRUE ) = 0;
    virtual AbstractScDataPilotDatabaseDlg * CreateScDataPilotDatabaseDlg (Window* pParent ,const ResId& rResId ) = 0; //add for ScDataPilotDatabaseDlg

    virtual AbstractScDataPilotSourceTypeDlg * CreateScDataPilotSourceTypeDlg ( Window* pParent, BOOL bEnableExternal, const ResId& rResId ) = 0; //add for ScDataPilotSourceTypeDlg

    virtual AbstractScDataPilotServiceDlg * CreateScDataPilotServiceDlg( Window* pParent, //add for ScDataPilotServiceDlg
                                                                        const com::sun::star::uno::Sequence<rtl::OUString>& rServices,
                                                                        const ResId& rResId ) = 0;

    virtual AbstractScDeleteCellDlg * CreateScDeleteCellDlg( Window* pParent, const ResId& rResId, BOOL bDisallowCellMove = FALSE ) = 0 ; //add for ScDeleteCellDlg

    virtual AbstractScDeleteContentsDlg * CreateScDeleteContentsDlg(Window* pParent,const ResId& rResId, //add for ScDeleteContentsDlg
                                                                 USHORT  nCheckDefaults = 0 ) = 0;
    virtual AbstractScFillSeriesDlg * CreateScFillSeriesDlg( Window*        pParent, //add for ScFillSeriesDlg
                                                            ScDocument& rDocument,
                                                            FillDir     eFillDir,
                                                            FillCmd     eFillCmd,
                                                            FillDateCmd eFillDateCmd,
                                                            String          aStartStr,
                                                            double          fStep,
                                                            double          fMax,
                                                            USHORT          nPossDir,
                                                            const ResId& rResId) = 0;

    virtual AbstractScGroupDlg * CreateAbstractScGroupDlg( Window* pParent, //add for ScGroupDlg
                                                            USHORT  nResId,
                                                            const ResId& rResId,
                                                            BOOL    bUnGroup = FALSE,
                                                            BOOL    bRows    = TRUE  ) = 0;

    virtual AbstractScInsertCellDlg * CreateScInsertCellDlg( Window* pParent, //add for ScInsertCellDlg
                                                                const ResId& rResId,
                                                            BOOL bDisallowCellMove = FALSE ) = 0;

    virtual AbstractScInsertContentsDlg * CreateScInsertContentsDlg( Window*        pParent, //add for ScInsertContentsDlg
                                                                    const ResId& rResId,
                                                                    USHORT          nCheckDefaults = 0,
                                                                    const String*   pStrTitle = NULL ) = 0;

    virtual AbstractScInsertTableDlg * CreateScInsertTableDlg ( Window* pParent, ScViewData& rViewData,  //add for ScInsertTableDlg
                                                                SCTAB nTabCount, bool bFromFile, const ResId& rResId) = 0;

    virtual AbstractScSelEntryDlg * CreateScSelEntryDlg ( Window* pParent, // add for ScSelEntryDlg
                                                            USHORT  nResId,
                                                        const String& aTitle,
                                                        const String& aLbTitle,
                                                                List&   aEntryList,
                                                            const ResId& rResId ) = 0;
    virtual AbstractScLinkedAreaDlg * CreateScLinkedAreaDlg (  Window* pParent, const ResId& rResId) = 0; //add for ScLinkedAreaDlg

    virtual AbstractScMetricInputDlg * CreateScMetricInputDlg (  Window*        pParent, //add for ScMetricInputDlg
                                                                USHORT      nResId,     // Ableitung fuer jeden Dialog!
                                                                long            nCurrent,
                                                                long            nDefault,
                                                                const ResId& rResId ,
                                                                FieldUnit       eFUnit    = FUNIT_MM,
                                                                USHORT      nDecimals = 2,
                                                                long            nMaximum  = 1000,
                                                                long            nMinimum  = 0,
                                                                long            nFirst    = 1,
                                                                long          nLast     = 100 ) = 0;

    virtual AbstractScMoveTableDlg * CreateScMoveTableDlg(  Window* pParent, const ResId& rResId ) = 0; //add for ScMoveTableDlg

    virtual AbstractScNameCreateDlg * CreateScNameCreateDlg ( Window * pParent, USHORT nFlags, const ResId& rResId ) = 0; //add for ScNameCreateDlg

    virtual AbstractScNamePasteDlg * CreateScNamePasteDlg ( Window * pParent, const ScRangeName* pList, //add for ScNamePasteDlg
                                                            const ResId& rResId , BOOL bInsList=TRUE ) = 0;

    virtual AbstractScPivotFilterDlg * CreateScPivotFilterDlg ( Window* pParent, //add for ScPivotFilterDlg
                                                                const SfxItemSet&   rArgSet, USHORT nSourceTab , const ResId& rResId ) = 0;

    virtual AbstractScDPFunctionDlg * CreateScDPFunctionDlg( Window* pParent, const ResId& rResId,
                                                                const ScDPLabelDataVec& rLabelVec,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScDPFuncData& rFuncData ) = 0;

    virtual AbstractScDPSubtotalDlg * CreateScDPSubtotalDlg( Window* pParent, const ResId& rResId,
                                                                ScDPObject& rDPObj,
                                                                const ScDPLabelData& rLabelData,
                                                                const ScDPFuncData& rFuncData,
                                                                const ScDPNameVec& rDataFields,
                                                                bool bEnableLayout ) = 0;

    virtual AbstractScDPShowDetailDlg * CreateScDPShowDetailDlg( Window* pParent, const ResId& rResId,
                                                                ScDPObject& rDPObj,
                                                                USHORT nOrient ) = 0;

    virtual AbstractScNewScenarioDlg * CreateScNewScenarioDlg ( Window* pParent, const String& rName, //add for ScNewScenarioDlg
                                                                const ResId& rResId,
                                                                BOOL bEdit = FALSE, BOOL bSheetProtected = FALSE ) = 0;
    virtual AbstractScShowTabDlg * CreateScShowTabDlg ( Window* pParent, const ResId& rResId ) = 0; //add for ScShowTabDlg

    virtual AbstractScStringInputDlg * CreateScStringInputDlg (  Window* pParent, //add for ScStringInputDlg
                                                                const String& rTitle,
                                                                const String& rEditTitle,
                                                                const String& rDefault,
                                                                ULONG nHelpId ,
                                                                const ResId& rResId ) = 0;
    virtual AbstractScImportOptionsDlg * CreateScImportOptionsDlg ( Window*                 pParent, //add for ScImportOptionsDlg
                                                                    const ResId& rResId,
                                                                    BOOL                    bAscii = TRUE,
                                                                    const ScImportOptions*  pOptions = NULL,
                                                                    const String*           pStrTitle = NULL,
                                                                    BOOL                    bMultiByte = FALSE,
                                                                    BOOL                    bOnlyDbtoolsEncodings = FALSE,
                                                                    BOOL                    bImport = TRUE ) = 0;

    virtual SfxAbstractTabDialog * CreateScAttrDlg( SfxViewFrame*    pFrame, //add for ScAttrDlg
                                                    Window*          pParent,
                                                    const SfxItemSet* pCellAttrs,
                                                    const ResId& rResId) = 0;

    virtual SfxAbstractTabDialog * CreateScHFEditDlg( SfxViewFrame*     pFrame, //add for ScHFEditDlg
                                                    Window*         pParent,
                                                    const SfxItemSet&   rCoreSet,
                                                    const String&       rPageStyle,
                                                    const ResId& rResId,
                                                    USHORT              nResId = RID_SCDLG_HFEDIT ) = 0;

    virtual SfxAbstractTabDialog * CreateScStyleDlg( Window*                pParent,//add for ScStyleDlg
                                                    SfxStyleSheetBase&  rStyleBase,
                                                    USHORT              nRscId,
                                                    const ResId& rResId) = 0;

    virtual SfxAbstractTabDialog * CreateScSubTotalDlg( Window*             pParent, //add for ScSubTotalDlg
                                                        const SfxItemSet*   pArgSet,
                                                        const ResId& rResId) = 0;

    virtual SfxAbstractTabDialog * CreateScCharDlg( Window* pParent, const SfxItemSet* pAttr,//add for ScCharDlg
                                                    const SfxObjectShell* pDocShell, const ResId& rResId ) = 0;

    virtual SfxAbstractTabDialog * CreateScParagraphDlg( Window* pParent, const SfxItemSet* pAttr ,//add for ScParagraphDlg
                                                            const ResId& rResId ) = 0;

    virtual SfxAbstractTabDialog * CreateScValidationDlg( Window* pParent, //add for ScValidationDlg
                                                        const SfxItemSet* pArgSet,const ResId& rResId  ) = 0;
    virtual SfxAbstractTabDialog * CreateScSortDlg( Window*          pParent, //add for ScSortDlg
                                                    const SfxItemSet* pArgSet,const ResId& rResId ) = 0;
    // for tabpage
    virtual CreateTabPage               GetTabPageCreatorFunc( USHORT nId ) = 0;
    virtual GetTabPageRanges            GetTabPageRangesFunc( USHORT nId ) = 0;
};
#endif

