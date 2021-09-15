/* This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <editeng/editeng.hxx>
#include <editeng/fontitem.hxx>
#include <unotools/lingucfg.hxx>

class SmEditEngine final : public EditEngine
{
public:
    SmEditEngine(SfxItemPool* pItemPool);
    SmEditEngine(const SmEditEngine&) = delete;

public:
    /**
      * Runs checkZoom and if true runs updateZoom
      */
    void executeZoom(EditView* pEditView = nullptr);

    /**
      * Sets up default font parameters for the item pool.
      */
    static void setSmItemPool(SfxItemPool* mpItemPool, const SvtLinguOptions& maLangOptions);

    // Deal with text scaling
private:
    sal_Int32 m_nOldZoom;
    sal_Int32 m_nNewZoom;
    sal_Int32 m_nDefaultFontSize;

    /**
      * Checks if the zoom of smeditwindow has changed.
      * m_nNewZoom is updated.
      * @return zoom has changed
      */
    bool checkZoom();

    /**
      * Updates the zoom of smeditwindow.
      * m_nOldZoom is set to m_nNewZoom.
      */

    void updateZoom();

    // Gather information for more complex tasks
private:
    ESelection m_aAllSelection;

    /**
      * Finds the ESelection which contains all the text.
      */
    void updateAllESelection();
};
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
