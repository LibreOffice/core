/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tptable.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-23 11:46:51 $
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

#ifndef SC_TPTABLE_HXX
#define SC_TPTABLE_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

//===================================================================

/** A vcl/NumericField that additionally supports empty text.
    @descr  Value 0 is set as empty text, and empty text is returned as 0. */
class EmptyNumericField : public NumericField
{
public:
    inline explicit     EmptyNumericField( Window* pParent, WinBits nWinStyle ) :
                            NumericField( pParent, nWinStyle ) {}
    inline explicit     EmptyNumericField( Window* pParent, const ResId& rResId ) :
                            NumericField( pParent, rResId ) {}

    virtual void        Modify();
    virtual void        SetValue( sal_Int64 nValue );
    virtual sal_Int64   GetValue() const;
};

//===================================================================

class ScTablePage : public SfxTabPage
{
public:
    static  SfxTabPage* Create          ( Window*           pParent,
                                          const SfxItemSet& rCoreSet );
    static  USHORT*     GetRanges       ();
    virtual BOOL        FillItemSet     ( SfxItemSet& rCoreSet );
    virtual void        Reset           ( const SfxItemSet& rCoreSet );
    virtual int         DeactivatePage  ( SfxItemSet* pSet = NULL );
    virtual void        DataChanged     ( const DataChangedEvent& rDCEvt );

private:
                    ScTablePage( Window* pParent, const SfxItemSet& rCoreSet );
    virtual         ~ScTablePage();

    void            ShowImage();

private:
    FixedLine       aFlPageDir;
    RadioButton     aBtnTopDown;
    RadioButton     aBtnLeftRight;
    FixedImage      aBmpPageDir;
    Image           aImgLeftRight;
    Image           aImgTopDown;
    Image           aImgLeftRightHC;
    Image           aImgTopDownHC;
    CheckBox        aBtnPageNo;
    NumericField    aEdPageNo;

    FixedLine       aFlPrint;
    CheckBox        aBtnHeaders;
    CheckBox        aBtnGrid;
    CheckBox        aBtnNotes;
    CheckBox        aBtnObjects;
    CheckBox        aBtnCharts;
    CheckBox        aBtnDrawings;
    CheckBox        aBtnFormulas;
    CheckBox        aBtnNullVals;

    FixedLine           aFlScale;
    FixedText           aFtScaleMode;
    ListBox             aLbScaleMode;
    FixedText           aFtScaleAll;
    MetricField         aEdScaleAll;
    FixedText           aFtScalePageWidth;
    EmptyNumericField   aEdScalePageWidth;
    FixedText           aFtScalePageHeight;
    EmptyNumericField   aEdScalePageHeight;
    FixedText           aFtScalePageNum;
    NumericField        aEdScalePageNum;

private:
    //------------------------------------
    // Handler:
    DECL_LINK( PageDirHdl,      RadioButton* );
    DECL_LINK( PageNoHdl,       CheckBox* );
    DECL_LINK( ScaleHdl,        ListBox* );
};

#endif // SC_TPTABLE_HXX
