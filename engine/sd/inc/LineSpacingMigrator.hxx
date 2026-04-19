/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "ModelTraverser.hxx"
#include <vcl/virdev.hxx>

#include <memory>
#include <unordered_map>

class SdDrawDocument;
class SdOutliner;
class SdrTextObj;
class OutlinerParaObject;

namespace sd
{
/// Migrate shapes from font metric based line spacing to font independent
/// line spacing (120% of font size base). This makes documents independent
/// of font metric variations across systems and formats. The proportional
/// line spacing percentage is adjusted so that the rendered line height
/// remains the same (as much as that is possible) after the switch.
class LineSpacingMigrator : public ModelTraverseHandler,
                            public std::enable_shared_from_this<LineSpacingMigrator>
{
public:
    static void migrate(SdDrawDocument* pDocument);
    void handleSdrObject(SdrObject* pObject) override;

private:
    double getConversionFactor(const OUString& rFontName);
    void migrateShape(SdrTextObj* pTextObject);
    bool migrateParagraph(OutlinerParaObject* pOutlinerParaObject, sal_Int32 nParagraph,
                          double fShapeFactor);

    SdOutliner* mpOutliner = nullptr;
    ScopedVclPtrInstance<VirtualDevice> mpVirtualDevice;
    std::unordered_map<OUString, double> maFactorCache;
};

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
