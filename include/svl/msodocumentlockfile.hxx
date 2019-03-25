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
#include <svl/lockfilecommon.hxx>
#include <svl/documentlockfile.hxx>

#include <com/sun/star/lang/XComponent.hpp>

namespace com
{
namespace sun
{
namespace star
{
namespace io
{
class XInputStream;
}
}
}
}
namespace com
{
namespace sun
{
namespace star
{
namespace io
{
class XOutputStream;
}
}
}
}

#define MSO_WORD_LOCKFILE_SIZE 162
#define MSO_EXCEL_AND_POWERPOINT_LOCKFILE_SIZE 165
#define MSO_USERNAME_MAX_LENGTH 52

namespace svt
{
/// Class implementing reading and writing MSO lockfiles.
class SVL_DLLPUBLIC MSODocumentLockFile : public GenDocumentLockFile
{
private:
    OUString m_sOrigURL;

    static bool isWordFormat(const OUString& aOrigURL);
    static bool isExcelFormat(const OUString& aOrigURL);
    static bool isPowerPointFormat(const OUString& aOrigURL);

protected:
    virtual void
    WriteEntryToStream(const LockFileEntry& aEntry,
                       const css::uno::Reference<css::io::XOutputStream>& xStream) override;

public:
    MSODocumentLockFile(const OUString& aOrigURL);
    virtual ~MSODocumentLockFile() override;

    /// Need to generate different lock file name for MSO.
    virtual OUString GenerateURL(const OUString& aOrigURL, const OUString& aPrefix) override;

    virtual LockFileEntry GetLockData() override;

    virtual void RemoveFile() override;

    static bool IsMSOSupportedFileFormat(const OUString& aURL);
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
