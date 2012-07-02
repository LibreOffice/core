/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef _VCL_AQUA_CORETEXT_SALCORETEXTFONTUTILS_HXX
#define _VCL_AQUA_CORETEXT_SALCORETEXTFONTUTILS_HXX

class CoreTextPhysicalFontFace;
class ImplDevFontList;

#include <boost/unordered_map.hpp>

#include <vcl/fontcapabilities.hxx>

#include "outfont.hxx"
#include "impfont.hxx"

class CoreTextPhysicalFontFace : public PhysicalFontFace
{
public:
    CoreTextPhysicalFontFace(const ImplDevFontAttributes&, CTFontRef font);
    virtual ~CoreTextPhysicalFontFace();
    virtual PhysicalFontFace* Clone() const;
    virtual ImplFontEntry* CreateFontInstance( FontSelectPattern& ) const;
    virtual sal_IntPtr GetFontId() const { return (sal_IntPtr)m_CTFontRef;};
    CTFontRef GetCTFont() const { return m_CTFontRef; };
    const ImplFontCharMap* GetImplFontCharMap();
    bool GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities);
    bool HasChar( sal_uInt32 cChar ) const;
    void ReadOs2Table();
    void ReadIosCmapEncoding();
    bool HasCJKSupport();
    bool GetRawFontData( std::vector<unsigned char>& rBuffer, bool* pJustCFF ) const;

private:
    void DetermineCJKSupport_OS2(CFDataRef rOS2Table);
    void DetermineCJKSupport_cmap(CFDataRef rCmapTable);
    CTFontRef m_CTFontRef;
    mutable const ImplFontCharMap* m_pCharMap;
    mutable vcl::FontCapabilities m_aFontCapabilities;
    mutable bool m_bHasOs2Table;
    mutable bool m_bOs2TableRead;
    mutable bool m_bCmapTableRead; // true if cmap encoding of Mac font is read
    mutable bool m_bHasCJKSupport; // #i78970# CJK fonts need extra leading
    mutable bool m_bFontCapabilitiesRead;
};

/* This class has the responsibility of assembling a list of CoreText
   fonts available on the system and enabling access to that list.
 */
class SystemFontList
{
public:
    SystemFontList();
    ~SystemFontList();

    void AnnounceFonts( ImplDevFontList& ) const;
    CoreTextPhysicalFontFace* GetFontDataFromRef( CTFontRef ) const;

private:
    typedef boost::unordered_map<CTFontRef,CoreTextPhysicalFontFace*> CoreTextFontContainer;
    CoreTextFontContainer m_aFontContainer;

    void InitGlyphFallbacks();
};

#endif  // _VCL_AQUA_CORETEXT_SALCORETEXTFONTUTILS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
