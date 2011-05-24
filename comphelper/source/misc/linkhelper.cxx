/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *        Caolán McNamara <caolanm@redhat.com> (Red Hat, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Caolán McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

#include <comphelper/linkhelper.hxx>
#include <osl/file.hxx>

namespace comphelper
{
    /** Resolve a file url if its a symbolic link, to a depth of nDepth
     *
     *  @return false on failure, true on success
     */
    bool resolveLink(rtl::OUString &rURL, int nDepth)
    {
        bool bReturn = false;

        osl::FileStatus status(osl_FileStatus_Mask_Type | osl_FileStatus_Mask_LinkTargetURL);
        osl::DirectoryItem item;
        while (osl::DirectoryItem::get(rURL, item) == osl::File::E_None)
        {
            if (--nDepth == 0)
                break;
            if (item.getFileStatus(status) != osl::File::E_None)
                break;
            if (status.getFileType() != osl::FileStatus::Link)
            {
                bReturn = true;
                break;
            }
            rURL = status.getLinkTargetURL();
        }

        return bReturn;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
