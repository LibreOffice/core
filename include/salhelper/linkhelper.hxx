/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SALHELPER_LINKHELPER_HXX
#define INCLUDED_SALHELPER_LINKHELPER_HXX

#include "rtl/ustring.hxx"
#include "osl/file.hxx"

namespace salhelper
{
    class SAL_WARN_UNUSED LinkResolver
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

        /** Resolve a file url if it's a symbolic link, to a maximum depth of
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
