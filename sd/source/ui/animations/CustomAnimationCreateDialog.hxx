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



#ifndef _SD_CUSTOMANIMATIONCREATEDIALOG_HXX
#define _SD_CUSTOMANIMATIONCREATEDIALOG_HXX

#include "CustomAnimationPreset.hxx"
#include <vcl/tabdlg.hxx>

enum PathKind { NONE, CURVE, POLYGON, FREEFORM };

class TabControl;
class OKButton;
class CancelButton;
class HelpButton;

namespace sd {

// --------------------------------------------------------------------

class CustomAnimationCreateTabPage;
class CustomAnimationPane;

class CustomAnimationCreateDialog : public TabDialog
{
    friend class CustomAnimationCreateTabPage;
public:
    CustomAnimationCreateDialog( ::Window* pParent, CustomAnimationPane* pPane, const std::vector< ::com::sun::star::uno::Any >& rTargets, bool bHasText, const ::rtl::OUString& rsPresetId, double fDuration );
    ~CustomAnimationCreateDialog();

    PathKind getCreatePathKind() const;
    CustomAnimationPresetPtr getSelectedPreset() const;
    double getSelectedDuration() const;

private:
    CustomAnimationCreateTabPage* getCurrentPage() const;
    void preview( const CustomAnimationPresetPtr& pPreset ) const;
    void setPosition();
    void storePosition();

    DECL_LINK( implActivatePagekHdl, Control* );
    DECL_LINK( implDeactivatePagekHdl, Control* );

private:
    CustomAnimationPane* mpPane;
    const std::vector< ::com::sun::star::uno::Any >& mrTargets;

    double mfDuration;
    bool mbIsPreview;

    TabControl* mpTabControl;
    OKButton* mpOKButton;
    CancelButton* mpCancelButton;
    HelpButton* mpHelpButton;

    CustomAnimationCreateTabPage* mpTabPages[5];
};

}

#endif // _SD_CUSTOMANIMATIONCREATEDIALOG_HXX
