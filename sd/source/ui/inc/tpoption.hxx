/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SD_TPOPTION_HXX
#define _SD_TPOPTION_HXX


#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <svtools/stdctrl.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/optgrid.hxx>

/*************************************************************************
|*
|* Optionen-Tab-Page: Snap
|*
\************************************************************************/
class SdTpOptionsSnap : public SvxGridTabPage
{
public:
            SdTpOptionsSnap( Window* pParent, const SfxItemSet& rInAttrs  );
            ~SdTpOptionsSnap();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

//    virtual void ActivatePage( const SfxItemSet& rSet );
//    virtual int  DeactivatePage( SfxItemSet* pSet );
};
/*************************************************************************
|*
|* Optionen-Tab-Page: Contents (Inhalte)
|*
\************************************************************************/
class SdTpOptionsContents : public SfxTabPage
{
private:
    FixedLine    aGrpDisplay;
    CheckBox    aCbxRuler;
    CheckBox    aCbxDragStripes;
    CheckBox    aCbxHandlesBezier;
    CheckBox    aCbxMoveOutline;


public:
            SdTpOptionsContents( Window* pParent, const SfxItemSet& rInAttrs  );
            ~SdTpOptionsContents();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );
};

/*************************************************************************
|*
|* Optionen-Tab-Page: View
|*
\************************************************************************/
class SdModule;
class SdTpOptionsMisc : public SfxTabPage
{
 friend class SdOptionsDlg;
 friend class SdModule;

private:
    FixedLine   aGrpText;
    CheckBox    aCbxQuickEdit;
    CheckBox    aCbxPickThrough;

    FixedLine   aGrpProgramStart;
    CheckBox    aCbxStartWithTemplate;

    FixedLine   aGrpSettings;
    CheckBox    aCbxMasterPageCache;
    CheckBox    aCbxCopy;
    CheckBox    aCbxMarkedHitMovesAlways;
    CheckBox    aCbxCrookNoContortion;

    FixedText   aTxtMetric;
    ListBox     aLbMetric;
    FixedText   aTxtTabstop;
    MetricField aMtrFldTabstop;

    CheckBox    aCbxStartWithActualPage;
    FixedLine   aGrpStartWithActualPage;
    FixedLine   aTxtCompatibility;
    CheckBox    aCbxUsePrinterMetrics;
    CheckBox    aCbxCompatibility;

    //Scale
    FixedLine       aGrpScale;
    FixedText       aFtScale;
    ComboBox        aCbScale;

    FixedText       aFtOriginal;
    FixedText       aFtEquivalent;

    FixedText       aFtPageWidth;
    FixedInfo       aFiInfo1;
    MetricField     aMtrFldOriginalWidth;

    FixedText       aFtPageHeight;
    FixedInfo       aFiInfo2;
    MetricField     aMtrFldOriginalHeight;

    MetricField     aMtrFldInfo1;
    MetricField     aMtrFldInfo2;

    sal_uInt32          nWidth;
    sal_uInt32          nHeight;
    String          aInfo1;
    String          aInfo2;

    SfxMapUnit          ePoolUnit;

    String          GetScale( sal_Int32 nX, sal_Int32 nY );
    bool            SetScale( const String& aScale, sal_Int32& rX, sal_Int32& rY );

    DECL_LINK( ModifyScaleHdl, void * );
    DECL_LINK( ModifyOriginalScaleHdl, void * );
    DECL_LINK( SelectMetricHdl_Impl, ListBox * );

    /** Enable or disable the controls in the compatibility section of the
        'general' tab page depending on whether there is at least one
        document.
    */
    void UpdateCompatibilityControls (void);

protected:
    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int DeactivatePage( SfxItemSet* pSet );

public:
            SdTpOptionsMisc( Window* pParent, const SfxItemSet& rInAttrs  );
            ~SdTpOptionsMisc();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    /** Hide Impress specific controls, make Draw specific controls visible
        and arrange the visible controls.  Do not call this method or the
        <member>SetImpressMode()</member> method more than once.
    */
    void SetDrawMode (void);

    /** Hide Draw specific controls, make Impress specific controls visible
        and arrange the visible controls.  Do not call this method or the
        <member>SetDrawMode()</member> method more than once.
    */
    void SetImpressMode (void);
    virtual void        PageCreated (SfxAllItemSet aSet);

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    using OutputDevice::SetDrawMode;

};


#endif // _SD_TPOPTION_HXX

