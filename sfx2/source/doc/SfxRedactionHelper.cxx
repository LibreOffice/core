/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SfxRedactionHelper.hxx>

#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>

#include <svl/eitem.hxx>
#include <svl/stritem.hxx>

#include <svtools/DocumentToGraphicRenderer.hxx>

#include <vcl/gdimtf.hxx>

bool SfxRedactionHelper::isRedactMode(const SfxRequest& rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    if (pArgs)
    {
        const SfxBoolItem* pIsRedactMode = rReq.GetArg<SfxBoolItem>(SID_IS_REDACT_MODE);
        if (pIsRedactMode && pIsRedactMode->GetValue())
            return true;
    }

    return false;
}

OUString SfxRedactionHelper::getStringParam(const SfxRequest& rReq, const sal_uInt16& nParamId)
{
    OUString sStringParam;

    const SfxItemSet* pArgs = rReq.GetArgs();
    if (!pArgs)
        return sStringParam;

    const SfxStringItem* pStringArg = rReq.GetArg<SfxStringItem>(nParamId);
    if (!pStringArg)
        return sStringParam;

    sStringParam = pStringArg->GetValue();
    return sStringParam;
}

void SfxRedactionHelper::getPageMetaFilesFromDoc(std::vector<GDIMetaFile>& aMetaFiles,
                                                 const sal_Int32& nPages,
                                                 DocumentToGraphicRenderer& aRenderer,
                                                 bool bIsWriter, bool bIsCalc)
{
    for (sal_Int32 nPage = 1; nPage <= nPages; ++nPage)
    {
        ::Size aDocumentSizePixel = aRenderer.getDocumentSizeInPixels(nPage);
        ::Point aLogicPos;
        ::Point aCalcPageLogicPos;
        ::Size aCalcPageContentSize;
        ::Size aLogic = aRenderer.getDocumentSizeIn100mm(nPage, &aLogicPos, &aCalcPageLogicPos,
                                                         &aCalcPageContentSize);
        // FIXME: This is a temporary hack. Need to figure out a proper way to derive this scale factor.
        ::Size aTargetSize(aDocumentSizePixel.Width() * 1.23, aDocumentSizePixel.Height() * 1.23);

        Graphic aGraphic = aRenderer.renderToGraphic(nPage, aDocumentSizePixel, aTargetSize,
                                                     COL_TRANSPARENT, true);
        auto& rGDIMetaFile = const_cast<GDIMetaFile&>(aGraphic.GetGDIMetaFile());

        // Set preferred map unit and size on the metafile, so the Shape size
        // will be correct in MM.
        MapMode aMapMode;
        aMapMode.SetMapUnit(MapUnit::Map100thMM);
        // FIXME: This is a temporary hack. Need to figure out a proper way to derive these magic numbers.
        if (bIsWriter)
            aMapMode.SetOrigin(::Point(-(aLogicPos.getX() - 512) * 1.53,
                                       -((aLogicPos.getY() - 501) * 1.53 + (nPage - 1) * 740)));
        else if (bIsCalc)
            rGDIMetaFile.Scale(0.566, 0.566);

        rGDIMetaFile.SetPrefMapMode(aMapMode);

        if (bIsCalc)
        {
            double aWidthRatio = static_cast<double>(aCalcPageContentSize.Width()) / aLogic.Width();
            // FIXME: Get rid of these magic numbers. Also watch for floating point rounding errors
            rGDIMetaFile.Move(-2400 + aCalcPageLogicPos.X() * (aWidthRatio - 0.0887),
                              -3300 + aCalcPageLogicPos.Y() * 0.64175);
        }

        rGDIMetaFile.SetPrefSize(bIsCalc ? aCalcPageContentSize : aLogic);

        aMetaFiles.push_back(rGDIMetaFile);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
