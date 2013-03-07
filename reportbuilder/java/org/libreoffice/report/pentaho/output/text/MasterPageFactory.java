/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package org.libreoffice.report.pentaho.output.text;

import org.libreoffice.report.pentaho.OfficeNamespaces;
import org.libreoffice.report.pentaho.model.OfficeMasterPage;
import org.libreoffice.report.pentaho.model.OfficeMasterStyles;
import org.libreoffice.report.pentaho.model.OfficeStyles;
import org.libreoffice.report.pentaho.model.PageLayout;
import org.libreoffice.report.pentaho.model.RawText;

import java.util.HashMap;
import java.util.Map;

import org.jfree.layouting.input.style.values.CSSNumericValue;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;
import org.jfree.report.util.AttributeNameGenerator;


/**
 * Todo: Document me!
 *
 * @since 14.03.2007
 */
public class MasterPageFactory
{

    private static class MasterPageFactoryKey
    {

        private final String template;
        private final String pageHeader;
        private final String pageFooter;

        public MasterPageFactoryKey(final String template,
                final String pageHeader,
                final String pageFooter)
        {
            this.template = template;
            this.pageHeader = pageHeader;
            this.pageFooter = pageFooter;
        }

        public boolean equals(final Object o)
        {
            if (this != o)
            {
                if (o == null || getClass() != o.getClass())
                {
                    return false;
                }

                final MasterPageFactoryKey that = (MasterPageFactoryKey) o;

                if (pageFooter != null ? !pageFooter.equals(
                        that.pageFooter) : that.pageFooter != null)
                {
                    return false;
                }
                if (pageHeader != null ? !pageHeader.equals(
                        that.pageHeader) : that.pageHeader != null)
                {
                    return false;
                }
                if (template != null ? !template.equals(
                        that.template) : that.template != null)
                {
                    return false;
                }
            }

            return true;
        }

        public int hashCode()
        {
            int result = (template != null ? template.hashCode() : 0);
            result = 31 * result + (pageHeader != null ? pageHeader.hashCode() : 0);
            result = 31 * result + (pageFooter != null ? pageFooter.hashCode() : 0);
            return result;
        }

        public String getTemplate()
        {
            return template;
        }

        public String getPageHeader()
        {
            return pageHeader;
        }

        public String getPageFooter()
        {
            return pageFooter;
        }
    }

    private static class PageLayoutKey
    {

        private final String templateName;
        private final CSSNumericValue headerHeight;
        private final CSSNumericValue footerHeight;

        public PageLayoutKey(final String templateName,
                final CSSNumericValue headerHeight,
                final CSSNumericValue footerHeight)
        {
            this.templateName = templateName;
            this.headerHeight = headerHeight;
            this.footerHeight = footerHeight;
        }

        public String getTemplateName()
        {
            return templateName;
        }

        public CSSNumericValue getHeaderHeight()
        {
            return headerHeight;
        }

        public CSSNumericValue getFooterHeight()
        {
            return footerHeight;
        }

        public boolean equals(final Object o)
        {
            if (this == o)
            {
                return true;
            }
            if (o == null || getClass() != o.getClass())
            {
                return false;
            }

            final PageLayoutKey key = (PageLayoutKey) o;

            if (footerHeight != null ? !footerHeight.equals(
                    key.footerHeight) : key.footerHeight != null)
            {
                return false;
            }
            if (headerHeight != null ? !headerHeight.equals(
                    key.headerHeight) : key.headerHeight != null)
            {
                return false;
            }
            return !(templateName != null ? !templateName.equals(
                    key.templateName) : key.templateName != null);

        }

        public int hashCode()
        {
            int result;
            result = (templateName != null ? templateName.hashCode() : 0);
            result = 31 * result + (headerHeight != null ? headerHeight.hashCode() : 0);
            result = 31 * result + (footerHeight != null ? footerHeight.hashCode() : 0);
            return result;
        }
    }
    // todo: Patch the page-layout ...
    private static final String DEFAULT_PAGE_NAME = "Default";
    private final OfficeMasterStyles predefinedStyles;
    private final AttributeNameGenerator masterPageNameGenerator;
    private final Map<MasterPageFactoryKey,OfficeMasterPage> masterPages;
    private final AttributeNameGenerator pageLayoutNameGenerator;
    private final Map<PageLayoutKey,String> pageLayouts;

    public MasterPageFactory(final OfficeMasterStyles predefinedStyles)
    {
        this.predefinedStyles = predefinedStyles;
        this.masterPages = new HashMap<MasterPageFactoryKey,OfficeMasterPage>();
        this.masterPageNameGenerator = new AttributeNameGenerator();
        this.pageLayouts = new HashMap<PageLayoutKey,String>();
        this.pageLayoutNameGenerator = new AttributeNameGenerator();
    }

    public OfficeMasterPage getMasterPage(final String template,
            final String pageHeader,
            final String pageFooter)
    {
        final MasterPageFactoryKey key =
                new MasterPageFactoryKey(template, pageHeader, pageFooter);
        return masterPages.get(key);
    }

    public boolean containsMasterPage(final String template,
            final String pageHeader,
            final String pageFooter)
    {
        final MasterPageFactoryKey key =
                new MasterPageFactoryKey(template, pageHeader, pageFooter);
        return masterPages.containsKey(key);
    }

    public OfficeMasterPage createMasterPage(final String template,
            final String pageHeader,
            final String pageFooter)
    {
        final MasterPageFactoryKey key =
                new MasterPageFactoryKey(template, pageHeader, pageFooter);
        final OfficeMasterPage cached = masterPages.get(key);
        if (cached != null)
        {
            return cached;
        }

        final String targetName = (masterPages.isEmpty()) ? "Standard" : template;

        OfficeMasterPage predef = predefinedStyles.getMasterPage(template);
        if (predef == null)
        {
            // This is a 'magic' name ..
            // todo: It could be that this should be called 'Standard' instead
            predef = predefinedStyles.getMasterPage(MasterPageFactory.DEFAULT_PAGE_NAME);
        }

        if (predef != null)
        {
            try
            {
                // derive
                final OfficeMasterPage derived = (OfficeMasterPage) predef.clone();
                return setupMasterPage(derived, targetName, pageHeader, pageFooter,
                        key);
            }
            catch (CloneNotSupportedException cne)
            {
                throw new IllegalStateException("Implementation error: Unable to derive page", cne);
            }
        }

        final OfficeMasterPage masterPage = new OfficeMasterPage();
        masterPage.setNamespace(OfficeNamespaces.STYLE_NS);
        masterPage.setType("master-page");
        return setupMasterPage(masterPage, targetName, pageHeader, pageFooter, key);
    }

    private OfficeMasterPage setupMasterPage(final OfficeMasterPage derived,
            final String targetName,
            final String pageHeader,
            final String pageFooter,
            final MasterPageFactoryKey key)
    {
        derived.setStyleName(masterPageNameGenerator.generateName(targetName));
        masterPages.put(key, derived);

        if (pageHeader != null)
        {
            final Section header = new Section();
            header.setNamespace(OfficeNamespaces.STYLE_NS);
            header.setType("header");
            header.addNode(new RawText(pageHeader));
            derived.addNode(header);
        }

        if (pageFooter != null)
        {
            final Section footer = new Section();
            footer.setNamespace(OfficeNamespaces.STYLE_NS);
            footer.setType("footer");
            footer.addNode(new RawText(pageFooter));
            derived.addNode(footer);
        }

        return derived;
    }

    public String createPageStyle(final OfficeStyles commonStyles,
            final CSSNumericValue headerHeight,
            final CSSNumericValue footerHeight)
    {
        final PageLayoutKey key =
                new PageLayoutKey(null, headerHeight, footerHeight);
        final PageLayout derived = new PageLayout();
        final String name = pageLayoutNameGenerator.generateName("autogenerated");
        derived.setStyleName(name);
        commonStyles.addPageStyle(derived);

        if (headerHeight != null)
        {
            final Section headerStyle = new Section();
            headerStyle.setNamespace(OfficeNamespaces.STYLE_NS);
            headerStyle.setType("header-style");
            derived.addNode(headerStyle);
            MasterPageFactory.applyHeaderFooterHeight(headerStyle, headerHeight);
        }

        if (footerHeight != null)
        {
            final Section footerStyle = new Section();
            footerStyle.setNamespace(OfficeNamespaces.STYLE_NS);
            footerStyle.setType("footer-style");
            derived.addNode(footerStyle);
            MasterPageFactory.applyHeaderFooterHeight(footerStyle, footerHeight);
        }
        pageLayouts.put(key, name);
        return name;
    }

    public String derivePageStyle(final String pageStyleTemplate,
            final OfficeStyles predefined,
            final OfficeStyles commonStyles,
            final CSSNumericValue headerHeight,
            final CSSNumericValue footerHeight)
            throws ReportProcessingException
    {
        if (pageStyleTemplate == null)
        {
            throw new NullPointerException("A style-name must be given");
        }

        final PageLayoutKey key =
                new PageLayoutKey(pageStyleTemplate, headerHeight, footerHeight);
        final String pageLayoutName = pageLayouts.get(key);
        if (pageLayoutName != null)
        {
            // there's already a suitable version included.
            return pageLayoutName;
        }

        final PageLayout original = predefined.getPageStyle(pageStyleTemplate);
        if (original == null)
        {
            throw new ReportProcessingException("Invalid page-layout '" + pageStyleTemplate + "', will not continue.");
        }

        try
        {
            final PageLayout derived = (PageLayout) original.clone();
            final String name = pageLayoutNameGenerator.generateName(
                    pageStyleTemplate);
            derived.setStyleName(name);
            commonStyles.addPageStyle(derived);

            if (headerHeight != null)
            {
                Section headerStyle = derived.getHeaderStyle();
                if (headerStyle == null)
                {
                    headerStyle = new Section();
                    headerStyle.setNamespace(OfficeNamespaces.STYLE_NS);
                    headerStyle.setType("header-style");
                    derived.addNode(headerStyle);
                }
                MasterPageFactory.applyHeaderFooterHeight(headerStyle, headerHeight);
            }

            if (footerHeight != null)
            {
                Section footerStyle = derived.getFooterStyle();
                if (footerStyle == null)
                {
                    footerStyle = new Section();
                    footerStyle.setNamespace(OfficeNamespaces.STYLE_NS);
                    footerStyle.setType("footer-style");
                    derived.addNode(footerStyle);
                }

                MasterPageFactory.applyHeaderFooterHeight(footerStyle, footerHeight);
            }
            pageLayouts.put(key, name);
            return name;
        }
        catch (CloneNotSupportedException e)
        {
            throw new IllegalStateException("Clone failed.", e);
        }
    }

    private static void applyHeaderFooterHeight(final Section headerFooterStyle,
            final CSSNumericValue style)
    {
        Element headerFooterProps = headerFooterStyle.findFirstChild(OfficeNamespaces.STYLE_NS, "header-footer-properties");
        if (headerFooterProps == null)
        {
            headerFooterProps = new Section();
            headerFooterProps.setNamespace(OfficeNamespaces.STYLE_NS);
            headerFooterProps.setType("header-footer-properties");
            headerFooterStyle.addNode(headerFooterProps);
        }
        headerFooterProps.setAttribute(OfficeNamespaces.SVG_NS, "height", style.getValue() + style.getType().getType());
    }
}
