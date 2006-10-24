#ifndef INCLUDED_DMAPPER_PROPERTYIDS_HXX
#include <PropertyIds.hxx>
#endif
#include <rtl/ustring.hxx>
#include <hash_map>

namespace dmapper{

struct OUStringHash
{
    unsigned long operator()(const PropertyIds& eId) const
    {
        return static_cast<unsigned long>(eId);
    }
};

struct OUStringEq
{
    bool operator() (const PropertyIds& rA, const PropertyIds& rB) const
    {
        return rA == rB;
    }
};

typedef ::std::hash_map< PropertyIds, ::rtl::OUString, OUStringHash, OUStringEq> PropertyNameMap_t;


//typedef std::map< PropertyIds, ::rtl::OUString > PropertyNameMap_t;
struct PropertyNameSupplier_Impl
{
    PropertyNameMap_t aNameMap;
};

/*-- 14.06.2006 11:01:31---------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyNameSupplier::PropertyNameSupplier() :
    m_pImpl(new PropertyNameSupplier_Impl)
{
}
/*-- 14.06.2006 11:01:32---------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyNameSupplier::~PropertyNameSupplier()
{
    delete m_pImpl;
}
/*-- 14.06.2006 11:01:32---------------------------------------------------

  -----------------------------------------------------------------------*/
const rtl::OUString& PropertyNameSupplier::GetName( PropertyIds eId )
{
    PropertyNameMap_t::iterator aIt = m_pImpl->aNameMap.find(eId);
    if(aIt == m_pImpl->aNameMap.end())
    {
        ::rtl::OUString sName;
        switch(eId)
        {
            case PROP_CHAR_WEIGHT:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharWeight")); break;
            case PROP_CHAR_POSTURE:    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharPosture")); break;
            case PROP_CHAR_STRIKEOUT:  sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharStrikeout")); break;
            case PROP_CHAR_CONTOURED:  sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharContoured")); break;
            case PROP_CHAR_SHADOWED:   sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharShadowed")); break;
            case PROP_CHAR_CASEMAP:    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharCaseMap")); break;
            case PROP_CHAR_COLOR:      sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharColor")); break;
            case PROP_CHAR_RELIEF:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharRelief")); break;
            case PROP_CHAR_UNDERLINE:  sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharUnderline")); break;
            case PROP_CHAR_UNDERLINE_COLOR:  sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharUnderlineColor")); break;
            case PROP_CHAR_UNDERLINE_HAS_COLOR:  sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharUnderlineHasColor")); break;
            case PROP_CHAR_WORD_MODE:  sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharWordMode")); break;
            case PROP_CHAR_ESCAPEMENT       :     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharEscapement")); break;
            case PROP_CHAR_ESCAPEMENT_HEIGHT:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharEscapementHeight")); break;
            case PROP_CHAR_HEIGHT:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharHeight")); break;
            case PROP_CHAR_HEIGHT_COMPLEX:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharHeightComplex")); break;
            case PROP_CHAR_LOCALE:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharLocale")); break;
            case PROP_CHAR_LOCALE_ASIAN:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharLocaleAsian")); break;
            case PROP_CHAR_LOCALE_COMPLEX:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharLocaleComplex")); break;
            case PROP_CHAR_WEIGHT_COMPLEX :  sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharWeightComplex")); break;
            case PROP_CHAR_POSTURE_COMPLEX:  sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharPostureComplex")); break;
            case PROP_CHAR_CHAR_KERNING:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharKerning")); break;
            case PROP_CHAR_AUTO_KERNING:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharAutoKerning")); break;
            case PROP_CHAR_SCALE_WIDTH:      sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharScaleWidth")); break;
            case PROP_CHAR_STYLE_NAME:      sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharStyleName")); break;
            case PROP_CHAR_FONT_NAME:                sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontName")); break;
            case PROP_CHAR_FONT_STYLE:               sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontStyle")); break;
            case PROP_CHAR_FONT_FAMILY:              sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontFamily")); break;
            case PROP_CHAR_FONT_CHAR_SET:            sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontCharSet")); break;
            case PROP_CHAR_FONT_PITCH:               sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontPitch")); break;
            case PROP_CHAR_FONT_NAME_ASIAN     :     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontNameAsian")); break;
            case PROP_CHAR_FONT_STYLE_ASIAN    :     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontStyleAsian")); break;
            case PROP_CHAR_FONT_FAMILY_ASIAN   :     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontFamilyAsian")); break;
            case PROP_CHAR_FONT_CHAR_SET_ASIAN :     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontCharSetAsian")); break;
            case PROP_CHAR_FONT_PITCH_ASIAN    :     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontPitchAsian")); break;
            case PROP_CHAR_FONT_NAME_COMPLEX   :     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontNameComplex")); break;
            case PROP_CHAR_FONT_STYLE_COMPLEX  :     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontStyleComplex")); break;
            case PROP_CHAR_FONT_FAMILY_COMPLEX:      sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontFamilyComplex")); break;
            case PROP_CHAR_FONT_CHAR_SET_COMPLEX:    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontCharSetComplex")); break;
            case PROP_CHAR_FONT_PITCH_COMPLEX:       sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontPitchComplex")); break;
            case PROP_CHAR_HIDDEN:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharHidden")); break;

//            case PROP_CHAR_:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Char")); break;
//            case PROP_CHAR_:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Char")); break;
//            case PROP_CHAR_:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Char")); break;

            case PROP_PARA_STYLE_NAME:      sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaStyleName")); break;
            case PROP_PARA_ADJUST:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaAdjust")); break;
            case PROP_PARA_LAST_LINE_ADJUST:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaLastLineAdjust")); break;
            case PROP_PARA_RIGHT_MARGIN     :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaLeftMargin")); break;
            case PROP_PARA_LEFT_MARGIN      :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaRightMargin")); break;
            case PROP_PARA_FIRST_LINE_INDENT:    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaFirstLineIndent")); break;
            case PROP_PARA_KEEP_TOGETHER:       sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaKeepTogether")); break;
            case PROP_PARA_TOP_MARGIN:       sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaTopMargin")); break;
            case PROP_PARA_BOTTOM_MARGIN:       sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaBottomMargin")); break;
            case PROP_PARA_IS_HYPHENATION:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaIsHyphenation")); break;
            case PROP_PARA_LINE_NUMBER_COUNT:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaLineNumberCount")); break;
            case PROP_PARA_IS_HANGING_PUNCTUATION: sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaIsHangingPunctuation")); break;
            case PROP_PARA_LINE_SPACING:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaLineSpacing")); break;
            case PROP_PARA_TAB_STOPS:       sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaTabStops")); break;
//            case PROP_PARA_:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Para")); break;
//            case PROP_PARA_:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Para")); break;
//            case PROP_PARA_:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Para")); break;
//            case PROP_PARA_:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Para")); break;
//            case PROP_PARA_:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Para")); break;
//            case PROP_PARA_:     sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Para")); break;
            case PROP_NUMBERING_LEVEL:  sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberingLevel")); break;
            case PROP_NUMBERING_RULES:  sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberingRules")); break;
            case PROP_NUMBERING_TYPE:   sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberingType")); break;
            case PROP_START_WITH:       sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StartWith")); break;
            case PROP_ADJUST:           sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Adjust")); break;
            case PROP_PARENT_NUMBERING: sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParentNumbering")); break;
            case PROP_RIGHT_MARGIN     :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RightMargin")); break;
            case PROP_LEFT_MARGIN      :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LeftMargin")); break;
            case PROP_FIRST_LINE_OFFSET:    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FirstLineOffset")); break;
            case PROP_LEFT_BORDER           :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LeftBorder"));break;
            case PROP_RIGHT_BORDER          :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RightBorder"));break;
            case PROP_TOP_BORDER            :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TopBorder"));break;
            case PROP_BOTTOM_BORDER         :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BottomBorder"));break;
            case PROP_LEFT_BORDER_DISTANCE  :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LeftBorderDistance")); break;
            case PROP_RIGHT_BORDER_DISTANCE :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RightBorderDistance")); break;
            case PROP_TOP_BORDER_DISTANCE   :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TopBorderDistance"));break;
            case PROP_BOTTOM_BORDER_DISTANCE:    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BottomBorderDistance")); break;
            case PROP_CURRENT_PRESENTATION  :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CurrentPresentation")); break;
            case PROP_IS_FIXED              :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsFixed")); break;
            case PROP_SUB_TYPE              :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SubType")); break;
            case PROP_FILE_FORMAT           :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FileFormat")); break;
            case PROP_HYPER_LINK_U_R_L      :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HyperLinkURL")); break;
            case PROP_NUMBER_FORMAT         :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberFormat")); break;
            case PROP_NAME                  :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name")); break;
            case PROP_IS_INPUT              :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsInput")); break;
            case PROP_HINT                  :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Hint")); break;
            case PROP_FULL_NAME             :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FullName")); break;
            case PROP_KEYWORDS              :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Keywords")); break;
            case PROP_DESCRIPTION           :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Description")); break;
            case PROP_MACRO_NAME            :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MacroName")); break;
            case PROP_SUBJECT               :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Subject")); break;
            case PROP_USER_DATA_TYPE        :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UserDataType")); break;
            case PROP_TITLE                 :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Title")); break;
            case PROP_CONTENT               :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Content")); break;
            case PROP_DATA_COLUMN_NAME      :    sName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataColumnName")); break;


        }
        ::std::pair<PropertyNameMap_t::iterator,bool> aInsertIt =
                m_pImpl->aNameMap.insert( PropertyNameMap_t::value_type( eId, sName ));
        if(aInsertIt.second)
            aIt = aInsertIt.first;
    }
    return aIt->second;
}
PropertyNameSupplier& PropertyNameSupplier::GetPropertyNameSupplier()
{
    static PropertyNameSupplier aNameSupplier;
    return aNameSupplier;
}

} //namespace dmapper
