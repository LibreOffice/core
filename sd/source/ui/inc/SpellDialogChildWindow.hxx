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



#ifndef SD_SPELL_DIALOG_CHILD_WINDOW_HXX
#define SD_SPELL_DIALOG_CHILD_WINDOW_HXX

#include <svx/SpellDialogChildWindow.hxx>


namespace sd {

class Outliner;

/** This derivation of the ::svx::SpellDialogChildWindow base class
    provides Draw and Impress specific implementations of
    GetNextWrongSentence() and ApplyChangedSentence().
*/
class SpellDialogChildWindow
    : public ::svx::SpellDialogChildWindow
{
public:
    SpellDialogChildWindow (
        ::Window* pParent,
        sal_uInt16 nId,
        SfxBindings* pBindings,
        SfxChildWinInfo* pInfo);
    virtual ~SpellDialogChildWindow (void);

    /** This method makes the one from the base class public so that
        it can be called from the view shell when one is created.
    */
    virtual void InvalidateSpellDialog (void);

    SFX_DECL_CHILDWINDOW(SpellDialogChildWindow);

protected:
    /** Iterate over the sentences in all text shapes and stop at the
        next sentence with spelling errors. While doing so the view
        mode may be changed and text shapes are set into edit mode.
    */
    virtual ::svx::SpellPortions GetNextWrongSentence( bool bRecheck );

    /** This method is responsible for merging corrections made in the
        spelling dialog back into the document.
    */
    virtual void ApplyChangedSentence(const ::svx::SpellPortions& rChanged, bool bRecheck);
    virtual void GetFocus (void);
    virtual void LoseFocus (void);

private:
    /** This outliner is used to do the main work of iterating over a
        document and finding sentences with spelling errors.
    */
    Outliner* mpSdOutliner;

    /** When this flag is <true/> then eventually we have to destroy
        the outliner in mpSdOutliner.
    */
    bool mbOwnOutliner;

    /** Provide an outliner in the mpSdOutliner data member.  When the
        view shell has changed since the last call this include the
        deletion/release of formerly created/obtained one prior to
        construction/obtaining of a new one.
    */
    void ProvideOutliner (void);
};

} // end of namespace ::sd

#endif
