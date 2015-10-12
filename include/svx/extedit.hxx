/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_EXTEDIT_HXX
#define INCLUDED_SVX_EXTEDIT_HXX

#include <svx/svxdllapi.h>
#include <svl/lstner.hxx>
#include <rtl/ustring.hxx>
#include <memory>

class Graphic;
class GraphicObject;
class FileChangedChecker;

class SVX_DLLPUBLIC SAL_WARN_UNUSED ExternalToolEdit
{
protected:
    OUString m_aFileName;

    ::std::unique_ptr<FileChangedChecker> m_pChecker;

public:

    ExternalToolEdit();
    virtual ~ExternalToolEdit();

    virtual void Update( Graphic& aGraphic ) = 0;
    void Edit(GraphicObject const*const pGraphic);

    void StartListeningEvent();

    static void HandleCloseEvent( ExternalToolEdit* pData );
};

class FmFormView;
class SdrObject;

class SVX_DLLPUBLIC SAL_WARN_UNUSED SdrExternalToolEdit
    : public ExternalToolEdit
    , public SfxListener
{
private:
    FmFormView * m_pView;
    SdrObject *  m_pObj;

    SAL_DLLPRIVATE virtual void Update(Graphic&) override;
    SAL_DLLPRIVATE virtual void Notify(SfxBroadcaster&, const SfxHint&) override;

public:
    SdrExternalToolEdit(FmFormView * pView, SdrObject * pObj);
};

#endif
