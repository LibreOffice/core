/*************************************************************************
 *
 *  $RCSfile: b3dtex.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2000-11-14 13:27:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _B3D_B3DTEX_HXX
#define _B3D_B3DTEX_HXX

#ifndef _SV_OPENGL_HXX
#include <vcl/opengl.hxx>
#endif

#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

#ifndef _SV_COLOR_HXX
#include <vcl/color.hxx>
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>
#endif

// Vorausdeklarationen
class BitmapReadAccess;
class BitmapColor;

/*************************************************************************
|*
|* define fuer die Ueberlebensdauer einer Textur
|*
\************************************************************************/

#define B3D_TEXTURE_LIFETIME                (200)

/*************************************************************************
|*
|* Long-Zeiger fuer OpenGL Texturdatenuebergabe
|*
\************************************************************************/

#ifdef WIN
typedef UINT8 huge* GL_UINT8;
#else
typedef UINT8*      GL_UINT8;
#endif

/*************************************************************************
|*
|* Art der Pixeldaten der Textur
|*
\************************************************************************/

enum Base3DTextureKind
{
    Base3DTextureLuminance = 1,
    Base3DTextureIntensity,
    Base3DTextureColor
};

/*************************************************************************
|*
|* Modus der Textur
|*
\************************************************************************/

enum Base3DTextureMode
{
    Base3DTextureReplace = 1,
    Base3DTextureModulate,
    Base3DTextureBlend
};

/*************************************************************************
|*
|* Filtermodus der Textur
|*
\************************************************************************/

enum Base3DTextureFilter
{
    Base3DTextureNearest = 1,
    Base3DTextureLinear
};

/*************************************************************************
|*
|* Wrapping-Modus
|*
\************************************************************************/

enum Base3DTextureWrap
{
    Base3DTextureClamp = 1,
    Base3DTextureRepeat,
    Base3DTextureSingle
};

/*************************************************************************
|*
|* Defines fuer Maskenbildung um Entscheidung innerhalb von ModifyColor
|* zu beschleunigen
|*
\************************************************************************/

#define B3D_TXT_KIND_LUM        0x00
#define B3D_TXT_KIND_INT        0x01
#define B3D_TXT_KIND_COL        0x02

#define B3D_TXT_MODE_REP        0x04
#define B3D_TXT_MODE_MOD        0x08
#define B3D_TXT_MODE_BND        0x0C

#define B3D_TXT_FLTR_NEA        0x10

/*************************************************************************
|*
|* Klassen fuer TexturAttribute beim Anfordern von Texturen
|*
\************************************************************************/

#define TEXTURE_ATTRIBUTE_TYPE_BITMAP           0x0000
#define TEXTURE_ATTRIBUTE_TYPE_GRADIENT         0x0001
#define TEXTURE_ATTRIBUTE_TYPE_HATCH            0x0002

class TextureAttributes
{
private:
    BOOL            mbGhosted;

public:
    TextureAttributes(BOOL bGhosted);

    virtual BOOL operator==(const TextureAttributes&) const;
    virtual UINT16 GetTextureAttributeType() const =0;

    BOOL GetGhostedAttribute() { return mbGhosted; }
};

class TextureAttributesBitmap : public TextureAttributes
{
private:
    Bitmap          aBitmapAttribute;

public:
    TextureAttributesBitmap(Bitmap aBmp, BOOL bGhosted);

    virtual BOOL operator==(const TextureAttributes&) const;
    virtual UINT16 GetTextureAttributeType() const;

    Bitmap GetBitmapAttribute() { return aBitmapAttribute; }
};

class TextureAttributesGradient : public TextureAttributes
{
private:
    void*           pFill;
    void*           pStepCount;

public:
    TextureAttributesGradient(void* pF, void *pSC, BOOL bGhosted);

    virtual BOOL operator==(const TextureAttributes&) const;
    virtual UINT16 GetTextureAttributeType() const;

    void* GetFillAttribute() { return pFill; }
    void* GetStepCountAttribute() { return pStepCount; }
};

class TextureAttributesHatch : public TextureAttributes
{
private:
    void*           pFill;

public:
    TextureAttributesHatch(void* pF, BOOL bGhosted);

    virtual BOOL operator==(const TextureAttributes&) const;
    virtual UINT16 GetTextureAttributeType() const;

    void* GetHatchFillAttribute() { return pFill; }
};

/*************************************************************************
|*
|* Klasse fuer Texturen in Base3D
|*
\************************************************************************/

class B3dTexture
{
protected:
    // Die Bitmap der Textur
    Bitmap                  aBitmap;
    BitmapReadAccess*       pReadAccess;

    // Attribute bei der Generierung
    TextureAttributes*      pAttributes;

    // Gibt die Haeufigkeit der Benutzung wieder
    UINT16                  nUsageCount;

    // Farbe fuer Base3DTextureBlend - Modus
    BitmapColor             aColBlend;

    // Farbe, wenn keine Textur an einer Stelle liegt
    BitmapColor             aColTexture;

    // Art der Textur
    Base3DTextureKind       eKind;

    // Modus der Textur
    Base3DTextureMode       eMode;

    // Filter
    Base3DTextureFilter     eFilter;

    // Wrapping-Modes fuer beide Freiheitsgrade
    Base3DTextureWrap       eWrapS;
    Base3DTextureWrap       eWrapT;

    // Entscheidungsvariable
    UINT8                   nSwitchVal;

    // Vorbestimmbare interne booleans
    unsigned                bTextureKindChanged : 1;

    // Konstruktor / Destruktor
    B3dTexture(TextureAttributes& rAtt,
        Bitmap& rBmp,
        Base3DTextureKind=Base3DTextureColor,
        Base3DTextureMode=Base3DTextureReplace,
        Base3DTextureFilter=Base3DTextureNearest,
        Base3DTextureWrap eS=Base3DTextureSingle,
        Base3DTextureWrap eT=Base3DTextureSingle);
    virtual ~B3dTexture();

    // Interne Zugriffsfunktion auf die BitMapFarben
    inline const BitmapColor GetBitmapColor(long nX, long nY);

    // Verwaltung UsageCount
    void Touch() { nUsageCount=B3D_TEXTURE_LIFETIME; }
    void DecrementUsageCount() { if(nUsageCount) nUsageCount--; }
    UINT16 GetUsageCount() { return nUsageCount; };
    void SetSwitchVal();

public:
    // Zugriff auf die Attribute der Textur
    TextureAttributes& GetAttributes();

    // Zugriff auf Bitmap
    Bitmap& GetBitmap() { return aBitmap; }
    const Size GetBitmapSize() { return aBitmap.GetSizePixel(); }

    // Texturfunktion
    void ModifyColor(Color& rCol, double fS, double fT);

    // Art der Pixeldaten lesen/bestimmen
    void SetTextureKind(Base3DTextureKind eNew);
    Base3DTextureKind GetTextureKind() { return eKind; }

    // Texturmodus lesen/bestimmen
    void SetTextureMode(Base3DTextureMode eNew);
    Base3DTextureMode GetTextureMode() { return eMode; }

    // Filtermodus lesen/bestimmen
    void SetTextureFilter(Base3DTextureFilter eNew);
    Base3DTextureFilter GetTextureFilter() { return eFilter; }

    // Wrapping fuer beide Freiheitsgrade lesen/bestimmen
    void SetTextureWrapS(Base3DTextureWrap eNew);
    Base3DTextureWrap GetTextureWrapS() { return eWrapS; }
    void SetTextureWrapT(Base3DTextureWrap eNew);
    Base3DTextureWrap GetTextureWrapT() { return eWrapT; }

    // Blend-Color lesen/bestimmen
    void SetBlendColor(Color rNew);
    Color GetBlendColor();

    // Textur-Ersatz-Color lesen/bestimmen
    void SetTextureColor(Color rNew);
    Color GetTextureColor();

protected:
    // Zugriff auf Konstruktor/Destruktor nur fuer die verwaltenden Klassen
    friend class Base3D;
    friend class Base3DOpenGL;
    friend class B3dTextureStore;
};

/*************************************************************************
|*
|* erweiterte Klasse fuer Texturen in Base3DOpenGL
|*
\************************************************************************/

class B3dTextureOpenGL : public B3dTexture
{
private:
    // Name dieser Textur in OpenGL
    GLuint                  nTextureName;

    // Konstruktor / Destruktor
    B3dTextureOpenGL(TextureAttributes& rAtt,
        Bitmap& rBmp,
        OpenGL& rOGL,
        Base3DTextureKind=Base3DTextureColor,
        Base3DTextureMode=Base3DTextureReplace,
        Base3DTextureFilter=Base3DTextureNearest,
        Base3DTextureWrap eS=Base3DTextureClamp,
        Base3DTextureWrap eT=Base3DTextureClamp);
    virtual ~B3dTextureOpenGL();

    // In OpenGL die Textur zerstoeren
    void DestroyOpenGLTexture(OpenGL&);

public:
    // Setze diese Textur in OpenGL als aktuelle Textur
    void MakeCurrentTexture(OpenGL&);

    // Erzeuge diese Textur als OpenGL-Textur
    void CreateOpenGLTexture(OpenGL&);

protected:
    // Zugriff auf Konstruktor/Destruktor nur fuer die verwaltenden Klassen
    friend class Base3D;
    friend class Base3DOpenGL;
    friend class B3dTextureStore;
};


#endif          // _B3D_B3DTEX_HXX
