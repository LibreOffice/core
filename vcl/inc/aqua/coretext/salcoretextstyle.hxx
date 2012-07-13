/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef _VCL_AQUA_CORETEXT_SALCORETEXTSTYLE_HXX
#define _VCL_AQUA_CORETEXT_SALCORETEXTSTYLE_HXX

#include "aqua/salgdicommon.hxx"

class FontSelectPattern;
class CoreTextPhysicalFontFace;

class CoreTextStyleInfo
{
public:
    CoreTextStyleInfo();
    ~CoreTextStyleInfo();
    CTFontRef GetFont() const { return m_CTFont; };
    CoreTextPhysicalFontFace* GetFontFace() const { return m_font_face; };
    long GetFontStretchedSize() const;
    float GetFontStretchFactor() const { return m_stretch_factor; };
    CTParagraphStyleRef GetParagraphStyle() const { return m_CTParagraphStyle; } ;
    CGSize    GetSize() const;
    CGColorRef GetColor() const { return m_color; } ;
    void SetColor(SalColor color);
    void SetColor(void);
    void SetFont(FontSelectPattern* requested_font);

private:
    bool m_fake_bold;
    bool m_fake_italic;
    CGAffineTransform m_matrix;
    float m_stretch_factor;
    float m_font_scale;
    float m_fake_dpi_scale;
    CTParagraphStyleRef m_CTParagraphStyle;
    CTFontRef m_CTFont;
    CGColorRef m_color;
    CoreTextPhysicalFontFace* m_font_face;

};

#endif // _VCL_AQUA_CORETEXT_SALCORETEXTSTYLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
