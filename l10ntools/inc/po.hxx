/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_L10NTOOLS_INC_PO_HXX
#define INCLUDED_L10NTOOLS_INC_PO_HXX

#include <fstream>
#include <memory>
#include <rtl/string.hxx>

class PoOfstream;
class PoIfstream;
class GenPoEntry;


/** Interface to use po entries in localization

    PoEntry based on GenPoEntry class which stores attributes
    of general po entry(see po.cxx). It makes easy to get/set
    all information needed to localize one english(US) string.
    It contains some basic checkings and some string
    transformations between po string and string used by
    localization tools.
*/
class PoEntry
{
private:

    std::unique_ptr<GenPoEntry> m_pGenPo;
    bool m_bIsInitialized;

public:

    friend class PoOfstream;
    friend class PoIfstream;

    enum TYPE { TTEXT, TQUICKHELPTEXT, TTITLE };
    enum Exception { NOSOURCFILE, NORESTYPE, NOGROUPID, NOSTRING, WRONGHELPTEXT };

                    PoEntry();
                    PoEntry( const OString& rSourceFile, const OString& rResType, const OString& rGroupId,
                             const OString& rLocalId, const OString& rHelpText, const OString& rText,
                             const TYPE eType = TTEXT );
                    ~PoEntry();

                    PoEntry( const PoEntry& rPo );
    PoEntry&        operator=( const PoEntry& rPo );

    OString const &  getSourceFile() const;      ///< Get name of file from which entry is extracted
    OString         getGroupId() const;
    OString         getLocalId() const;
    OString         getResourceType() const;    ///< Get the type of component from which entry is extracted
    TYPE            getType() const;            ///< Get the type of entry
    OString const & getMsgId() const;
    OString const & getMsgStr() const;
    bool            isFuzzy() const;

    /// Check whether po-s belong to the same localization component
    static bool     IsInSameComp(const PoEntry& rPo1,const PoEntry& rPo2);
    static OString  genKeyId(const OString& rGenerator);

};

/** Interface to work with header of po/pot files

    This class stores information which is in header of
    a po file. It's main function to generate header to
    template po files(pot).
*/
class PoHeader
{
private:

    GenPoEntry* m_pGenPo;
    bool m_bIsInitialized;

public:

    friend class PoOfstream;
    friend class PoIfstream;

                    PoHeader( const OString& rExtSrc ); ///< Template Constructor
                    ~PoHeader();
                    PoHeader(const PoHeader&) = delete;
    PoHeader&       operator=(const PoHeader&) = delete;
};

/// Interface to write po entry to files as output streams
class PoOfstream
{
private:

    std::ofstream   m_aOutPut;
    bool            m_bIsAfterHeader;

public:

    enum OpenMode { TRUNC, APP };

            PoOfstream();
            PoOfstream(const OString& rFileName, OpenMode aMode = TRUNC );
            ~PoOfstream();
            PoOfstream(const PoOfstream&) = delete;
    PoOfstream& operator=(const PoOfstream&) = delete;
    bool    isOpen() const  { return m_aOutPut.is_open(); }

    void    open(const OString& rFileName, OpenMode aMode = TRUNC );
    void    close();
    void    writeHeader(const PoHeader& rHeader);
    void    writeEntry(const PoEntry& rPo);
};

/// Interface to read po entry from files as input streams
class PoIfstream
{
private:

    std::ifstream   m_aInPut;
    bool            m_bEof;

public:

    class Exception : public std::exception { };

            PoIfstream();
            PoIfstream( const OString& rFileName );
            ~PoIfstream();
            PoIfstream(const PoIfstream&) = delete;
    PoIfstream& operator=(const PoIfstream&) = delete;
    bool    isOpen() const  { return m_aInPut.is_open(); }
    bool    eof() const     { return m_bEof; }

    void    open(const OString& rFileName);
    void    close();
    void    readEntry(PoEntry& rPo);
};

#endif // INCLUDED_L10NTOOLS_INC_PO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
