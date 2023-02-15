/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVL_MSODOCUMENTLOCKFILE_HXX
#define INCLUDED_SVL_MSODOCUMENTLOCKFILE_HXX

#include <svl/svldllapi.h>
#include <svl/documentlockfile.hxx>

#define MSO_WORD_LOCKFILE_SIZE 162
#define MSO_EXCEL_AND_POWERPOINT_LOCKFILE_SIZE 165
#define MSO_USERNAME_MAX_LENGTH 52

namespace svt
{
/// Class implementing reading and writing MSO lockfiles.
class SVL_DLLPUBLIC MSODocumentLockFile final : public GenDocumentLockFile
{
private:
    enum class AppType
    {
        Word,
        Excel,
        PowerPoint
    };
    static AppType getAppType(std::u16string_view sOrigURL);
    AppType m_eAppType;

    virtual void
    WriteEntryToStream(std::unique_lock<std::mutex>& rGuard, const LockFileEntry& aEntry,
                       const css::uno::Reference<css::io::XOutputStream>& xStream) override;

    virtual css::uno::Reference<css::io::XInputStream>
    OpenStream(std::unique_lock<std::mutex>& rGuard) override;

    virtual LockFileEntry GetLockDataImpl(std::unique_lock<std::mutex>& rGuard) override;

public:
    MSODocumentLockFile(std::u16string_view aOrigURL);
    virtual ~MSODocumentLockFile() override;

    virtual void RemoveFile() override;

    static bool IsMSOSupportedFileFormat(std::u16string_view aURL);
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
