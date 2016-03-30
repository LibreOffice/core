/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_NOTEBOOKBAR_SFXNOTEBOOKBAR_HXX
#define INCLUDED_SFX2_NOTEBOOKBAR_SFXNOTEBOOKBAR_HXX

#include <sfx2/dllapi.h>

class SfxBindings;

namespace sfx2 {

/** Helpers for easier access to NotebookBar via the sfx2 infrastructure.
*/
class SFX2_DLLPUBLIC SfxNotebookBar
{
public:
    /// Function to be called from the sdi's ExecMethod.
    static void ExecMethod(SfxBindings& rBindings);

    /// Function to be called from the sdi's StateMethod.
    static void StateMethod(SfxBindings& rBindings, const OUString& rUIFile);
};

} // namespace sfx2

#endif // INCLUDED_SFX2_NOTEBOOKBAR_SFXNOTEBOOKBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
