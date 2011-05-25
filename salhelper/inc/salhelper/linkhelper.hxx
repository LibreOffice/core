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
#ifndef _SALHELPER_LINKHELPER_HXX
#define _SALHELPER_LINKHELPER_HXX

#include <rtl/ustring.hxx>
#include <osl/file.hxx>

namespace salhelper
{
    class LinkResolver
    {
    public:
        osl::FileStatus m_aStatus;

        LinkResolver(sal_uInt32 nMask)
            : m_aStatus(nMask |
                        osl_FileStatus_Mask_FileURL |
                        osl_FileStatus_Mask_Type |
                        osl_FileStatus_Mask_LinkTargetURL)
        {
        }

        /** Resolve a file url if its a symbolic link, to a maximum depth of
         *  nDepth and fill in m_aStatus with the requested ctor flags
         *
         *  @return osl::FileBase::E_None on success
         *
         *  @see DirectoryItem::getFileStatus
         */
        osl::FileBase::RC fetchFileStatus(const rtl::OUString &rURL,
            int nDepth = 128)
        {
            //In an ideal world this wouldn't be inline, but I want to use this
            //in jvmfwk hence salhelper, but salhelper is .map controlled and
            //getting all the mangled names right is a misery, moving it over
            //to visibility markup would drop per-symbol versioning
            osl::FileBase::RC eReturn;

            osl::DirectoryItem item;
            rtl::OUString sURL(rURL);
            while ((eReturn = osl::DirectoryItem::get(sURL, item))
                  == osl::File::E_None)
            {
                if (--nDepth == 0)
                {
                    eReturn = osl::FileBase::E_MULTIHOP;
                    break;
                }
                eReturn = item.getFileStatus(m_aStatus);
                if (eReturn != osl::File::E_None)
                    break;
                if (m_aStatus.getFileType() != osl::FileStatus::Link)
                {
                    eReturn = osl::FileBase::E_None;
                    break;
                }
                sURL = m_aStatus.getLinkTargetURL();
            }

            return eReturn;
        }
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
