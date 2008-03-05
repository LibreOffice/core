/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StyleUtilities.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:35:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


package com.sun.star.report.pentaho.output;

import java.util.HashSet;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.DataStyle;
import com.sun.star.report.pentaho.model.FontFaceDeclsSection;
import com.sun.star.report.pentaho.model.FontFaceElement;
import com.sun.star.report.pentaho.model.OfficeStyle;
import com.sun.star.report.pentaho.model.OfficeStyles;
import com.sun.star.report.pentaho.model.OfficeStylesCollection;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.structure.Element;
import org.jfree.report.util.AttributeNameGenerator;
import org.jfree.util.Log;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
 * @since 13.03.2007
 */
public class StyleUtilities
{
    private static final String STYLE = "style";

  private StyleUtilities()
  {
  }

  /**
   * Copies the specififed style (keyed by its family and name) into the current styles collection. This copies the
   * style and all inherited styles into the target collection. Inherited common styles will be always be added to the
   * common collection (which will be written into the 'styles.xml' later).
   * <p/>
   * This method does nothing if the specified style already exists in the styles collection.
   *
   * @param styleFamily      the family of the style to copy
   * @param styleName        the unique name of the style.
   * @param stylesCollection the current styles collection
   * @param commonCollection the global styles collection
   * @param predefCollection the predefined styles from where to copy the styles.
   * @throws ReportProcessingException if the style copying failed.
   */
  public static void copyStyle(final String styleFamily,
                               final String styleName,
                               final OfficeStylesCollection stylesCollection,
                               final OfficeStylesCollection commonCollection,
                               final OfficeStylesCollection predefCollection)
      throws ReportProcessingException
  {
    copyStyle(styleFamily, styleName, stylesCollection,
        commonCollection, predefCollection, new HashSet());
  }

  /**
   * Copies the specififed style (keyed by its family and name) into the current styles collection. This copies the
   * style and all inherited styles into the target collection. Inherited common styles will be always be added to the
   * common collection (which will be written into the 'styles.xml' later).
   * <p/>
   * This method does nothing if the specified style already exists in the styles collection.
   *
   * @param styleFamily        the family of the style to copy
   * @param styleName          the unique name of the style.
   * @param stylesCollection   the current styles collection
   * @param commonCollection   the global styles collection
   * @param predefCollection   the predefined styles from where to copy the styles.
   * @param inheritanceTracker a collection of all styles that have been touched. This is used to prevent infinite
   *                           loops and duplicates.
   * @throws ReportProcessingException if the style copying failed.
   */
  private static void copyStyle(final String styleFamily,
                                final String styleName,
                                final OfficeStylesCollection stylesCollection,
                                final OfficeStylesCollection commonCollection,
                                final OfficeStylesCollection predefCollection,
                                final HashSet inheritanceTracker)
      throws ReportProcessingException
  {
    if (inheritanceTracker.contains(styleName))
    {
      return;
    }
    inheritanceTracker.add(styleName);

    if (stylesCollection.containsStyle(styleFamily, styleName) ||
        commonCollection.getCommonStyles().containsStyle(styleFamily, styleName))
    {
      // fine, there's already a copy of the stylesheet.
      return;
    }

    final OfficeStyle predefCommonStyle =
        predefCollection.getCommonStyles().getStyle(styleFamily, styleName);
    if (predefCommonStyle != null)
    {
      // so we have an style from the predefined collection.
      // copy it an add it to the current stylescollection
      final OfficeStyles commonStyles = commonCollection.getCommonStyles();

      copyStyleInternal(predefCommonStyle, commonStyles, stylesCollection,
          commonCollection, predefCollection, styleFamily, inheritanceTracker);
      return;
    }

    final OfficeStyle predefAutoStyle =
        predefCollection.getAutomaticStyles().getStyle(styleFamily, styleName);
    if (predefAutoStyle != null)
    {
      // so we have an style from the predefined collection.
      // copy it an add it to the current stylescollection
      final OfficeStyles autoStyles = stylesCollection.getAutomaticStyles();
      copyStyleInternal(predefAutoStyle, autoStyles, stylesCollection,
          commonCollection, predefCollection, styleFamily, inheritanceTracker);
      return;
    }

    // There is no automatic style either. Now this means that someone
    // messed up the fileformat. Lets create a new empty style for this.
    final OfficeStyle autostyle = new OfficeStyle();
    autostyle.setNamespace(OfficeNamespaces.STYLE_NS);
    autostyle.setType(STYLE);
    autostyle.setStyleFamily(styleFamily);
    autostyle.setStyleName(styleName);

    final OfficeStyles autoStyles = stylesCollection.getAutomaticStyles();
    autoStyles.addStyle(autostyle);
  }

  private static OfficeStyle copyStyleInternal(
      final OfficeStyle predefCommonStyle,
      final OfficeStyles styles,
      final OfficeStylesCollection stylesCollection,
      final OfficeStylesCollection commonCollection,
      final OfficeStylesCollection predefCollection,
      final String styleFamily,
      final HashSet inheritanceTracker)
      throws ReportProcessingException
  {
    try
    {
      final OfficeStyle preStyle = (OfficeStyle) predefCommonStyle.clone();
      styles.addStyle(preStyle);
      performFontFaceProcessing(preStyle, stylesCollection, predefCollection);
      performDataStyleProcessing(preStyle, stylesCollection, predefCollection);

      // Lookup the parent style ..
      final String styleParent = preStyle.getStyleParent();
      final OfficeStyle inherited =
          stylesCollection.getStyle(styleFamily, styleParent);
      if (inherited != null)
      {
        // OK, recurse (and hope that we dont run into an infinite loop) ..
        copyStyle(styleFamily, styleParent, stylesCollection,
            commonCollection, predefCollection, inheritanceTracker);
      }
      else if (styleParent != null)
      {
        Log.warn("Inconsistent styles: " + styleFamily + ":" + styleParent + " does not exist.");
      }
      return preStyle;
    }
    catch (CloneNotSupportedException e)
    {
      throw new ReportProcessingException
          ("Failed to derive a stylesheet", e);
    }
  }


  private static void performFontFaceProcessing
      (final OfficeStyle style,
       final OfficeStylesCollection stylesCollection,
       final OfficeStylesCollection predefCollection)
      throws ReportProcessingException
  {
    final Element textProperties = style.getTextProperties();
    if (textProperties == null)
    {
      return;
    }

    try
    {
      final FontFaceDeclsSection currentFonts = stylesCollection.getFontFaceDecls();
      final FontFaceDeclsSection predefFonts = predefCollection.getFontFaceDecls();

      final String fontName = (String)
          textProperties.getAttribute(OfficeNamespaces.STYLE_NS, "font-name");
      if (fontName != null)
      {
        if (currentFonts.containsFont(fontName) == false)
        {
          final FontFaceElement element = predefFonts.getFontFace(fontName);
          if (element != null)
          {
            currentFonts.addFontFace((FontFaceElement) element.clone());
          }
        }
      }

      final String fontNameAsian = (String)
          textProperties.getAttribute(OfficeNamespaces.STYLE_NS,
              "font-name-asian");
      if (fontNameAsian != null)
      {
        if (currentFonts.containsFont(fontNameAsian) == false)
        {
          final FontFaceElement element = predefFonts.getFontFace(
              fontNameAsian);
          if (element != null)
          {
            currentFonts.addFontFace((FontFaceElement) element.clone());
          }
        }
      }

      final String fontNameComplex = (String)
          textProperties.getAttribute(OfficeNamespaces.STYLE_NS,
              "font-name-complex");
      if (fontNameComplex != null)
      {
        if (currentFonts.containsFont(fontNameComplex) == false)
        {
          final FontFaceElement element = predefFonts.getFontFace(
              fontNameComplex);
          if (element != null)
          {
            currentFonts.addFontFace((FontFaceElement) element.clone());
          }
        }
      }
    }
    catch (CloneNotSupportedException e)
    {
      throw new ReportProcessingException("Failed to clone font-face element");
    }
  }

  private static void performDataStyleProcessing
      (final OfficeStyle style,
       final OfficeStylesCollection stylesCollection,
       final OfficeStylesCollection predefCollection)
      throws ReportProcessingException
  {
    final Object attribute = style.getAttribute(OfficeNamespaces.STYLE_NS, "data-style-name");
    if (attribute == null)
    {
      // the easy case: It has no number style at all.
      return;
    }

    final String styleName = String.valueOf(attribute);
    if (stylesCollection.getAutomaticStyles().containsDataStyle(styleName))
    {
      return;
    }
    if (stylesCollection.getCommonStyles().containsDataStyle(styleName))
    {
      return;
    }


    try
    {
      final OfficeStyles automaticStyles = predefCollection.getAutomaticStyles();
      final DataStyle autoDataStyle = automaticStyles.getDataStyle(styleName);
      if (autoDataStyle != null)
      {
        final DataStyle derivedStyle = (DataStyle) autoDataStyle.clone();
        stylesCollection.getAutomaticStyles().addDataStyle(derivedStyle);
        return;
      }
      final OfficeStyles commonStyles = predefCollection.getCommonStyles();
      final DataStyle commonDataStyle = commonStyles.getDataStyle(styleName);
      if (commonDataStyle != null)
      {
        final DataStyle derivedStyle = (DataStyle) commonDataStyle.clone();
        stylesCollection.getCommonStyles().addDataStyle(derivedStyle);
        return;
      }

      Log.warn("Dangling data style: " + styleName);
    }
    catch (CloneNotSupportedException e)
    {
      throw new ReportProcessingException("Failed to copy style. This should not have happened.");
    }
  }

  /**
   * Derives the named style. If the style is a common style, a new automatic style is generated and inserted into the
   * given stylesCollection. If the named style is an automatic style, the style is copied and inserted as new automatic
   * style.
   * <p/>
   * After the style has been created, the style's inheritance hierarchy will be copied as well.
   * <p/>
   * If there is no style with the given name and family, a new empty automatic style will be created.
   *
   * @param styleFamily      the family of the style to copy
   * @param styleName        the unique name of the style.
   * @param stylesCollection the current styles collection
   * @param commonCollection the global styles collection
   * @param predefCollection the predefined styles from where to copy the styles.
   * @param generator        the style-name-generator of the current report-target
   * @return the derived style instance.
   * @throws ReportProcessingException if the style copying failed.
   */
  public static OfficeStyle deriveStyle(final String styleFamily,
                                        final String styleName,
                                        final OfficeStylesCollection stylesCollection,
                                        final OfficeStylesCollection commonCollection,
                                        final OfficeStylesCollection predefCollection,
                                        final AttributeNameGenerator generator)
      throws ReportProcessingException
  {
    if (styleFamily == null)
    {
      throw new NullPointerException("StyleFamily must not be null");
    }
    if (styleName != null)
    {

      final OfficeStyle currentAuto =
          stylesCollection.getAutomaticStyles().getStyle(styleFamily,
              styleName);
      if (currentAuto != null)
      {
        // handle an automatic style ..
        final OfficeStyle derivedStyle =
            deriveAutomaticStyle(currentAuto, styleFamily, styleName,
                generator, commonCollection, predefCollection);
        stylesCollection.getAutomaticStyles().addStyle(derivedStyle);
        return derivedStyle;
      }

      final OfficeStyle currentCommon =
          stylesCollection.getCommonStyles().getStyle(styleFamily, styleName);
      if (currentCommon != null)
      {
        // handle an common style ..
        final OfficeStyle derivedStyle =
            deriveCommonStyle(currentCommon, styleFamily, styleName,
                generator, commonCollection, predefCollection);
        stylesCollection.getAutomaticStyles().addStyle(derivedStyle);
        return derivedStyle;
      }

//      final OfficeStyle commonAuto =
//          commonCollection.getAutomaticStyles().getStyle(styleFamily,
//              styleName);
//      if (commonAuto != null)
//      {
//        // handle an automatic style ..
//        final OfficeStyle derivedStyle =
//            deriveAutomaticStyle(commonAuto, styleFamily, styleName,
//                generator, commonCollection, predefCollection);
//        stylesCollection.getAutomaticStyles().addStyle(derivedStyle);
//        return derivedStyle;
//      }

      final OfficeStyle commonCommon =
          commonCollection.getCommonStyles().getStyle(styleFamily, styleName);
      if (commonCommon != null)
      {
        // handle an common style ..
        final OfficeStyle derivedStyle =
            deriveCommonStyle(commonCommon, styleFamily, styleName,
                generator, commonCollection, predefCollection);
        stylesCollection.getAutomaticStyles().addStyle(derivedStyle);
        return derivedStyle;
      }

      final OfficeStyle predefAuto =
          predefCollection.getAutomaticStyles().getStyle(styleFamily,
              styleName);
      if (predefAuto != null)
      {
        // handle an automatic style ..
        final OfficeStyle derivedStyle =
            deriveAutomaticStyle(predefAuto, styleFamily, styleName,
                generator, commonCollection, predefCollection);
        stylesCollection.getAutomaticStyles().addStyle(derivedStyle);
        return derivedStyle;
      }

      final OfficeStyle predefCommon =
          predefCollection.getCommonStyles().getStyle(styleFamily, styleName);
      if (predefCommon != null)
      {
        // handle an common style ..
        final OfficeStyle derivedStyle =
            deriveCommonStyle(predefCommon, styleFamily, styleName,
                generator, commonCollection, predefCollection);
        stylesCollection.getAutomaticStyles().addStyle(derivedStyle);
        return derivedStyle;
      }
    }

    // No such style. Create a new one ..
    final OfficeStyle autostyle = new OfficeStyle();
    autostyle.setNamespace(OfficeNamespaces.STYLE_NS);
    autostyle.setType(STYLE);
    autostyle.setStyleFamily(styleFamily);
    if (styleName != null)
    {
      autostyle.setStyleName(styleName);
    }
    else
    {
      autostyle.setStyleName(generator.generateName("derived_anonymous"));
    }

    final OfficeStyles autoStyles = stylesCollection.getAutomaticStyles();
    autoStyles.addStyle(autostyle);
    return autostyle;
  }

  private static OfficeStyle deriveCommonStyle(final OfficeStyle commonStyle,
                                               final String styleFamily,
                                               final String styleName,
                                               final AttributeNameGenerator nameGenerator,
                                               final OfficeStylesCollection commonCollection,
                                               final OfficeStylesCollection predefCollection)
      throws ReportProcessingException
  {
    final OfficeStyle autostyle = new OfficeStyle();
    autostyle.setNamespace(OfficeNamespaces.STYLE_NS);
    autostyle.setType(STYLE);
    autostyle.setStyleFamily(styleFamily);
    autostyle.setStyleName(nameGenerator.generateName("derived_" + styleName));
    autostyle.setStyleParent(styleName);

    // now copy the common style ..
    final OfficeStyles commonStyles = commonCollection.getCommonStyles();
    if (commonStyles.containsStyle(styleFamily, styleName) == false)
    {
      copyStyleInternal(commonStyle, commonStyles,
          commonCollection, commonCollection, predefCollection,
          styleFamily, new HashSet());
    }
    return autostyle;
  }

  private static OfficeStyle deriveAutomaticStyle(final OfficeStyle commonStyle,
                                                  final String styleFamily,
                                                  final String styleName,
                                                  final AttributeNameGenerator nameGenerator,
                                                  final OfficeStylesCollection commonCollection,
                                                  final OfficeStylesCollection predefCollection)
      throws ReportProcessingException
  {
    try
    {
      final OfficeStyle autostyle = (OfficeStyle) commonStyle.clone();
      autostyle.setNamespace(OfficeNamespaces.STYLE_NS);
      autostyle.setType(STYLE);
      autostyle.setStyleFamily(styleFamily);
      autostyle.setStyleName
          (nameGenerator.generateName("derived_auto_" + styleName));


      final String parent = autostyle.getStyleParent();
      if (parent != null)
      {
        copyStyle(styleFamily, parent, commonCollection, commonCollection,
            predefCollection);
      }
      return autostyle;
    }
    catch (CloneNotSupportedException e)
    {
      throw new ReportProcessingException(
          "Deriving the style failed. Clone error: ", e);
    }
  }

  public static String queryStyle (final OfficeStylesCollection predefCollection,
                                   final String styleFamily,
                                   final String styleName,
                                   final String sectionName,
                                   final String propertyNamespace,
                                   final String propertyName)
  {
    return queryStyle(predefCollection, styleFamily,
        styleName, sectionName, propertyNamespace, propertyName, new HashSet());
  }

  private static String queryStyle (final OfficeStylesCollection predefCollection,
                                   final String styleFamily,
                                   final String styleName,
                                   final String sectionName,
                                   final String propertyNamespace,
                                   final String propertyName,
                                   final HashSet seenStyles)
  {
    if (seenStyles.contains(styleName))
    {
      return null;
    }
    seenStyles.add(styleName);

    final OfficeStyle style = predefCollection.getStyle(styleFamily, styleName);
    if (style == null)
    {
      return null; // no such style
    }
    final Element section = style.findFirstChild(OfficeNamespaces.STYLE_NS, sectionName);
    if (section != null)
    {
      final Object attribute = section.getAttribute(propertyNamespace, propertyName);
      if (attribute != null)
      {
        return String.valueOf(attribute);
      }
    }
    final String parent = style.getStyleParent();
    if (parent == null)
    {
      return null;
    }
    return queryStyle(predefCollection, styleFamily, parent, sectionName, propertyNamespace, propertyName, seenStyles);
  }


}
