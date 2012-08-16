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
package com.sun.star.wizards.agenda;

import java.util.*;


import com.sun.star.awt.TextEvent;
import com.sun.star.beans.PropertyValue;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNamed;
import com.sun.star.document.XDocumentProperties;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XTerminateListener;
import com.sun.star.i18n.NumberFormatIndex;
import com.sun.star.lang.Locale;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.table.XCell;
import com.sun.star.table.XTableRows;
import com.sun.star.text.*;
import com.sun.star.uno.Any;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XNumberFormatsSupplier;
import com.sun.star.util.XNumberFormatter;
import com.sun.star.util.XSearchDescriptor;
import com.sun.star.util.XSearchable;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.NumberFormatter;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.document.OfficeDocument;
import com.sun.star.wizards.text.TextDocument;
import com.sun.star.wizards.text.TextSectionHandler;
import com.sun.star.wizards.ui.UnoDialog2;
import com.sun.star.wizards.ui.event.DataAware;

/**
 * 
 * The classes here implement the whole document-functionality of the agenda wizard: 
 * the live-preview and the final "creation" of the document, when the user clicks "finish". <br/>
 * <br/>
 * <h2>Some terminology:<h2/>
 * items are names or headings. we don't make any distinction.
 * 
 * <br/>
 * The Agenda Template is used as general "controller" of the whole document, whereas the
 * two child-classes ItemsTable and TopicsTable control the item tables (note plural!) and the
 * topics table (note singular).
 * <br/>   <br/>
 * Other small classes are used to abstract the handling of cells and text and we
 * try to use them as components.
 * <br/><br/>
 * We tried to keep the Agenda Template as flexible as possible, though there
 * must be many limitations, because it is generated dynamically.<br/><br/>
 * To keep the template flexible the following decisions were made:<br/>
 * 1. Item tables.<br/>
 * 1.a. there might be arbitrary number of Item tables.<br/>
 * 1.b. Item tables design (bordewr, background) is arbitrary.<br/>
 * 1.c. Items text styles are individual, and use stylelist styles with predefined names.<br/>
 * As result the following limitations:<br/>
 * Pairs of Name->value for each item.<br/>
 * Tables contain *only* those pairs.<br/>
 * 2. Topics table.<br/>
 * 2.a. arbitrary structure.<br/>
 * 2.b. design is arbitrary.<br/>
 * As result the following limitations:<br/>
 * No column merge is allowed.<br/>
 * One compolsary Heading row.<br/>
 * <br/><br/>
 * To let the template be flexible, we use a kind of "detection": we look where
 * the items are read the design of each table, reaplying it after writing the
 * table.
 * <br/><br/>
 * A note about threads:<br/>
 * Many methods here are synchronized, in order to avoid colission made by
 * events fired too often.
 * @author rpiterman
 *  
 */
public class AgendaTemplate extends TextDocument implements TemplateConsts, DataAware.Listener
{

    /**
     * resources.
     */
    AgendaWizardDialogResources resources;
    /**
     * data model. This keeps the status of the agenda document, and
     * every redraw is done according to this data.
     * Exception: topic data is written programatically, event-oriented.
     */
    CGAgenda agenda;
    /**
     * the UNO Text Document serrvice
     */
    Object document;
    /**
     * Service Factory
     */
    XMultiServiceFactory docMSF;
    /**
     * The template-filename of the current template.
     * Since we often re-link section and the break the link,
     * inorder to restore them, we need a template to link to.
     * This is practically an identicall copy of the current template. 
     */
    String template;
    /** 
     * used for common operations on sections.
     */
    TextSectionHandler textSectionHandler;
    /**
     * a component loader.
     */
    XComponentLoader xComponentLoader;
    /**
     * an array containing all ItemTable object (which control each an Items 
     * Table in the document. 
     */
    ItemsTable[] itemsTables;
    /**
     * the controller of the topics table.
     */
    Topics topics;
    /**
     *  Stores reusable OOo Placeholder TextFields to insert to the document.
     */
    Map<String, AgendaItem> itemsCache;
    /**
     * This map is used to find which tables contains a certain Item, so
     * the keys are the different Items, the Objects are the ItemTable controllers.
     * When an Item must be redrawn (because the user checked or uncheced it),
     * the controller is retrieved from this Map, and a redraw is issued on this controller.
     */
    Map<String, ItemsTable> itemsMap = new HashMap<String, ItemsTable>(11);
    /**
     * A temporary variable used to list all items and map them.
     */
    List<XTextRange> _allItems = new ArrayList<XTextRange>();
    /** 
     * keep a reference on some static items in the document, 
     * so when their content is changed (through the user), we
     * can just reference them and set their text.
     */
    TextElement teTitle, teDate, teTime, teLocation;
    XTextRange trTitle, trDate, trTime, trLocation;
    /**
     * used to format the date / time.
     */
    int dateFormat, timeFormat;
    XNumberFormatter dateFormatter, timeFormatter;
    /**
     * used to transfare time from VCL to UNO.
     */
    long docNullTime;
    Calendar calendar;
    /**
     * used to set the document title property (step 6).
     */
    private XDocumentProperties m_xDocProps;

    /**
     * loads the given template, and analyze its structure.
     * @param templateURL
     * @param topics
     * @see #initialize()
     * @see #initializeData(List)
     */
    public synchronized void load(String templateURL, List<PropertyValue[]> topics)
    {
        template = calcTemplateName(templateURL);
        document = loadAsPreview(templateURL, false);
        docMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, document);
        xFrame.getComponentWindow().setEnable(false);
        xTextDocument.lockControllers();
        initialize();
        initializeData(topics);
        xTextDocument.unlockControllers();
    }

    /**
     * The agenda templates are in format of aw-XXX.ott
     * the templates name is then XXX.ott.
     * This method calculates it.
     * @param url
     * @return the template name without the "aw-" at the beginning.
     */
    private String calcTemplateName(String url)
    {
        return FileAccess.connectURLs(FileAccess.getParentDir(url), FileAccess.getFilename(url).substring(3));
    }

    /**
     * synchronize the document to the model.<br/>
     * this method rewrites all titles, item tables , and the topics table-
     * thus synchronizing the document to the data model (CGAgenda).
     * @param topicsData since the model does not contain Topics
     * information (it is only actualized on save) the given list
     * supplies this information.
     */
    private void initializeData(List<PropertyValue[]> topicsData)
    {
        for (int i = 0; i < itemsTables.length; i++)
        {
            try
            {
                itemsTables[i].write(PropertyNames.EMPTY_STRING);
            }
            catch (Exception ex)
            {
                ex.printStackTrace();
            }
        }
        redrawTitle("txtTitle");
        redrawTitle("txtDate");
        redrawTitle("txtTime");
        redrawTitle("cbLocation");

        topics.writeAll(topicsData);

        setTemplateTitle(agenda.cp_TemplateName);

    }

    /**
     * redraws/rewrites the table which contains the given item
     * This method is called when the user checks/unchecks an item.
     * The table is being found, in which the item is, and redrawn.
     * @param itemName
     */
    public synchronized void redraw(String itemName)
    {
        try
        {
            // get the table in which the item is...
            Object itemsTable =
                    itemsMap.get(itemName);
            // rewrite the table.
            ((ItemsTable) itemsTable).write(null);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    /**
     * update the documents title property to the given title
     * @param newTitle new title.
     */
    synchronized void setTemplateTitle(String newTitle)
    {
        m_xDocProps.setTitle(newTitle);
    }

    /**
     * constructor. The document is *not* loaded here.
     * only some formal members are set.
     * @param xmsf_ service factory.
     * @param agenda_ the data model (CGAgenda)
     * @param resources_ resources.
     */
    AgendaTemplate(XMultiServiceFactory xmsf_, CGAgenda agenda_, AgendaWizardDialogResources resources_, XTerminateListener listener)
    {
        super(xmsf_, listener, "WIZARD_LIVE_PREVIEW");

        agenda = agenda_;
        resources = resources_;

        if (itemsCache == null)
        {
            initItemsCache();
        }
        _allItems = null;

    }

    /**
     * checks the data model if the
     * item corresponding to the given string should be shown
     * @param itemName a string representing an Item (name or heading).
     * @return true if the model specifies that the item should be displayed.
     */
    boolean isShowItem(String itemName)
    {
        if (itemName.equals(FILLIN_MEETING_TYPE))
        {
            return agenda.cp_ShowMeetingType;
        }
        else if (itemName.equals(FILLIN_READ))
        {
            return agenda.cp_ShowRead;
        }
        else if (itemName.equals(FILLIN_BRING))
        {
            return agenda.cp_ShowBring;
        }
        else if (itemName.equals(FILLIN_NOTES))
        {
            return agenda.cp_ShowNotes;
        }
        else if (itemName.equals(FILLIN_FACILITATOR))
        {
            return agenda.cp_ShowFacilitator;
        }
        else if (itemName.equals(FILLIN_TIMEKEEPER))
        {
            return agenda.cp_ShowTimekeeper;
        }
        else if (itemName.equals(FILLIN_NOTETAKER))
        {
            return agenda.cp_ShowNotetaker;
        }
        else if (itemName.equals(FILLIN_PARTICIPANTS))
        {
            return agenda.cp_ShowAttendees;
        }
        else if (itemName.equals(FILLIN_CALLED_BY))
        {
            return agenda.cp_ShowCalledBy;
        }
        else if (itemName.equals(FILLIN_OBSERVERS))
        {
            return agenda.cp_ShowObservers;
        }
        else if (itemName.equals(FILLIN_RESOURCE_PERSONS))
        {
            return agenda.cp_ShowResourcePersons;
        }
        else
        {
            throw new IllegalArgumentException("No such item");
        }
    }

    /**
     * itemsCache is a Map containing all agenda item. These are object which 
     * "write themselfs" to the table, given a table cursor.
     * A cache is used in order to reuse the objects, instead of recreate them.
     * This method fills the cache will all items objects (names and headings).
     */
    private void initItemsCache()
    {
        itemsCache = new HashMap<String, AgendaItem>(11);

        XMultiServiceFactory xmsf = UnoRuntime.queryInterface(XMultiServiceFactory.class, document);
        // Headings

        itemsCache.put(FILLIN_MEETING_TYPE,
                new AgendaItem(FILLIN_MEETING_TYPE, new TextElement(resources.itemMeetingType, STYLE_MEETING_TYPE),
                new PlaceholderElement(STYLE_MEETING_TYPE_TEXT, resources.reschkMeetingTitle_value, resources.resPlaceHolderHint, xmsf)));

        itemsCache.put(FILLIN_BRING,
                new AgendaItem(FILLIN_BRING, new TextElement(resources.itemBring, STYLE_BRING),
                new PlaceholderElement(STYLE_BRING_TEXT, resources.reschkBring_value, resources.resPlaceHolderHint, xmsf)));

        itemsCache.put(FILLIN_READ,
                new AgendaItem(FILLIN_READ, new TextElement(resources.itemRead, STYLE_READ),
                new PlaceholderElement(STYLE_READ_TEXT, resources.reschkRead_value, resources.resPlaceHolderHint, xmsf)));

        itemsCache.put(FILLIN_NOTES,
                new AgendaItem(FILLIN_NOTES, new TextElement(resources.itemNote, STYLE_NOTES),
                new PlaceholderElement(STYLE_NOTES_TEXT, resources.reschkNotes_value, resources.resPlaceHolderHint, xmsf)));

        // Names

        itemsCache.put(FILLIN_CALLED_BY,
                new AgendaItem(FILLIN_CALLED_BY, new TextElement(resources.itemCalledBy, STYLE_CALLED_BY),
                new PlaceholderElement(STYLE_CALLED_BY_TEXT, resources.reschkConvenedBy_value, resources.resPlaceHolderHint, xmsf)));

        itemsCache.put(FILLIN_FACILITATOR,
                new AgendaItem(FILLIN_FACILITATOR, new TextElement(resources.itemFacilitator, STYLE_FACILITATOR),
                new PlaceholderElement(STYLE_FACILITATOR_TEXT, resources.reschkPresiding_value, resources.resPlaceHolderHint, xmsf)));

        itemsCache.put(FILLIN_PARTICIPANTS,
                new AgendaItem(FILLIN_PARTICIPANTS, new TextElement(resources.itemAttendees, STYLE_PARTICIPANTS),
                new PlaceholderElement(STYLE_PARTICIPANTS_TEXT, resources.reschkAttendees_value, resources.resPlaceHolderHint, xmsf)));

        itemsCache.put(FILLIN_NOTETAKER,
                new AgendaItem(FILLIN_NOTETAKER, new TextElement(resources.itemNotetaker, STYLE_NOTETAKER),
                new PlaceholderElement(STYLE_NOTETAKER_TEXT, resources.reschkNoteTaker_value, resources.resPlaceHolderHint, xmsf)));

        itemsCache.put(FILLIN_TIMEKEEPER,
                new AgendaItem(FILLIN_TIMEKEEPER, new TextElement(resources.itemTimekeeper, STYLE_TIMEKEEPER),
                new PlaceholderElement(STYLE_TIMEKEEPER_TEXT, resources.reschkTimekeeper_value, resources.resPlaceHolderHint, xmsf)));

        itemsCache.put(FILLIN_OBSERVERS,
                new AgendaItem(FILLIN_OBSERVERS, new TextElement(resources.itemObservers, STYLE_OBSERVERS),
                new PlaceholderElement(STYLE_OBSERVERS_TEXT, resources.reschkObservers_value, resources.resPlaceHolderHint, xmsf)));

        itemsCache.put(FILLIN_RESOURCE_PERSONS,
                new AgendaItem(FILLIN_RESOURCE_PERSONS, new TextElement(resources.itemResource, STYLE_RESOURCE_PERSONS),
                new PlaceholderElement(STYLE_RESOURCE_PERSONS_TEXT, resources.reschkResourcePersons_value, resources.resPlaceHolderHint, xmsf)));

    }

    /**
     * Initializes a template.<br/>
     * This method does the following tasks:<br/>
     * Get a Time and Date format for the document, and retrieve the null date of the document (which is
     * document-specific).<br/>
     * Initializes the Items Cache map.
     * Analyses the document:<br/>
     * -find all "fille-ins" (apear as &gt;xxx&lt; in the document).
     * -analyze all items sections (and the tables in them).
     * -locate the titles and actualize them
     * -analyze the topics table
     */
    private void initialize()
    {
        /*
         * Get the default locale of the document, and create the date and time formatters.
         */
        XMultiServiceFactory docMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, document);
        try
        {
            Object defaults = docMSF.createInstance("com.sun.star.text.Defaults");
            Locale l = (Locale) Helper.getUnoStructValue(defaults, "CharLocale");

            java.util.Locale jl = new java.util.Locale(
                    l.Language, l.Country, l.Variant);

            calendar = Calendar.getInstance(jl);

            XNumberFormatsSupplier nfs = UnoRuntime.queryInterface(XNumberFormatsSupplier.class, document);
            Object formatSettings = nfs.getNumberFormatSettings();
            com.sun.star.util.Date date = (com.sun.star.util.Date) Helper.getUnoPropertyValue(formatSettings, "NullDate");

            calendar.set(date.Year, date.Month - 1, date.Day);

            docNullTime = JavaTools.getTimeInMillis(calendar);

            dateFormat = NumberFormatter.getNumberFormatterKey(nfs, NumberFormatIndex.DATE_SYSTEM_LONG);
            timeFormat = NumberFormatter.getNumberFormatterKey(nfs, NumberFormatIndex.TIME_HHMM);


            dateFormatter = NumberFormatter.createNumberFormatter(xMSF, nfs);
            timeFormatter = NumberFormatter.createNumberFormatter(xMSF, nfs);
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            throw new NullPointerException("Fatal Error: could not initialize locale or date/time formats.");
        }

        /*
         * get the document properties object.
         */
        m_xDocProps = OfficeDocument.getDocumentProperties(document);

        initItemsCache();
        initializeItems();
        initializeTitles();
        initializeItemsSections();
        XMultiServiceFactory xMultiServiceFactory = UnoRuntime.queryInterface(XMultiServiceFactory.class, document);
        textSectionHandler = new TextSectionHandler(xMultiServiceFactory, UnoRuntime.queryInterface(XTextDocument.class, document));
        initializeTopics();
        _allItems.clear();
        _allItems = null;
    }

    /**
     * locates the titles (name, location, date, time) and saves a reference to thier Text ranges.
     *
     */
    private void initializeTitles()
    {
        XTextRange item = null;

        XMultiServiceFactory xmsf = UnoRuntime.queryInterface(XMultiServiceFactory.class, document);

        for (int i = 0; i < _allItems.size(); i++)
        {
            item = _allItems.get(i);
            String text = item.getString().trim().toLowerCase();
            if (text.equals(FILLIN_TITLE))
            {

                teTitle = new PlaceholderTextElement(item, resources.resPlaceHolderTitle, resources.resPlaceHolderHint, xmsf);
                trTitle = item;
                _allItems.remove(i--);
            }
            else if (text.equals(FILLIN_DATE))
            {
                teDate = new PlaceholderTextElement(item, resources.resPlaceHolderDate, resources.resPlaceHolderHint, xmsf);
                trDate = item;
                _allItems.remove(i--);
            }
            else if (text.equals(FILLIN_TIME))
            {
                teTime = new PlaceholderTextElement(item, resources.resPlaceHolderTime, resources.resPlaceHolderHint, xmsf);
                trTime = item;
                _allItems.remove(i--);
            }
            else if (text.equals(FILLIN_LOCATION))
            {
                teLocation = new PlaceholderTextElement(item, resources.resPlaceHolderLocation, resources.resPlaceHolderHint, xmsf);
                trLocation = item;
                _allItems.remove(i--);
            }
        }
    }

    private void initializeTopics()
    {
        topics = new Topics();
    }

    private void initializeItems()
    {
        _allItems = searchFillInItems();
    }

    /**
     * searches the document for items in the format "&gt;*&lt;"
     * @return a vector containing the XTextRanges of the found items
     */
    private List<XTextRange> searchFillInItems()
    {
        try
        {
            XSearchable xSearchable = UnoRuntime.queryInterface(XSearchable.class, document);
            XSearchDescriptor sd = xSearchable.createSearchDescriptor();
            sd.setSearchString("<[^>]+>");
            sd.setPropertyValue("SearchRegularExpression", Boolean.TRUE);
            sd.setPropertyValue("SearchWords", Boolean.TRUE);

            XIndexAccess ia = xSearchable.findAll(sd);

            List<XTextRange> l = new ArrayList<XTextRange>(ia.getCount());
            for (int i = 0; i < ia.getCount(); i++)
            {
                try
                {
                    l.add(UnoRuntime.queryInterface(XTextRange.class, ia.getByIndex(i)));
                }
                catch (Exception ex)
                {
                    System.err.println("Nonfatal Error in finding fillins.");
                }
            }
            return l;
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            throw new IllegalArgumentException("Fatal Error: Loading template failed: searching fillins failed");
        }
    }

    /**
     * analyze the item sections in the template. delegates the analyze of each table to the
     * ItemsTable class.
     */
    private void initializeItemsSections()
    {
        String[] sections = getSections(document, TemplateConsts.SECTION_ITEMS);

        // for each section - there is a table...
        itemsTables = new ItemsTable[sections.length];

        for (int i = 0; i < itemsTables.length; i++)
        {
            try
            {
                itemsTables[i] = new ItemsTable(getSection(sections[i]), getTable(sections[i]));
            }
            catch (Exception ex)
            {
                ex.printStackTrace();
                throw new IllegalArgumentException("Fatal Error while initialilzing Template: items table in section " + sections[i]);
            }
        }

    }

    private String[] getSections(Object document, String s)
    {
        XTextSectionsSupplier xTextSectionsSupplier = UnoRuntime.queryInterface(XTextSectionsSupplier.class, document);
        String[] allSections = xTextSectionsSupplier.getTextSections().getElementNames();
        return getNamesWhichStartWith(allSections, s);
    }

    Object getSection(String name) throws NoSuchElementException, WrappedTargetException
    {
        XTextSectionsSupplier xTextSectionsSupplier = UnoRuntime.queryInterface(XTextSectionsSupplier.class, document);
        return ((Any) (xTextSectionsSupplier.getTextSections().getByName(name))).getObject();
    }

    Object getTable(String name) throws NoSuchElementException, WrappedTargetException
    {
        XTextTablesSupplier xTextTablesSupplier = UnoRuntime.queryInterface(XTextTablesSupplier.class, document);
        return ((Any) xTextTablesSupplier.getTextTables().getByName(name)).getObject();
    }

    /**
     * implementation of DataAware.Listener, is
     * called when title/date/time or location are
     * changed.
     */
    public synchronized void eventPerformed(Object param)
    {
        TextEvent te = (TextEvent) param;
        String controlName = (String) Helper.getUnoPropertyValue(
                UnoDialog2.getModel(te.Source),
                PropertyNames.PROPERTY_NAME);
        redrawTitle(controlName);

    }

    private synchronized void redrawTitle(String controlName)
    {
        if (controlName.equals("txtTitle"))
        {
            writeTitle(teTitle, trTitle, agenda.cp_Title);
        }
        else if (controlName.equals("txtDate"))
        {
            writeTitle(teDate, trDate, getDateString(agenda.cp_Date));
        }
        else if (controlName.equals("txtTime"))
        {
            writeTitle(teTime, trTime, getTimeString(agenda.cp_Time));
        }
        else if (controlName.equals("cbLocation"))
        {
            writeTitle(teLocation, trLocation, agenda.cp_Location);
        }
        else
        {
            throw new IllegalArgumentException("No such title control...");
        }
    }

    private void writeTitle(TextElement te, XTextRange tr, String text)
    {
        te.text = (text == null ? PropertyNames.EMPTY_STRING : text);
        te.write(tr);
    }
    private static long DAY_IN_MILLIS = (24 * 60 * 60 * 1000);

    private String getDateString(String d)
    {
        if (d == null || d.equals(PropertyNames.EMPTY_STRING))
        {
            return PropertyNames.EMPTY_STRING;
        }
        int date = Integer.parseInt(d);
        calendar.clear();
        calendar.set(date / 10000,
                (date % 10000) / 100 - 1,
                date % 100);

        long date1 = JavaTools.getTimeInMillis(calendar);
        /*
         * docNullTime and date1 are in millis, but
         * I need a day...
         */
        double daysDiff = (date1 - docNullTime) / DAY_IN_MILLIS + 1;

        return dateFormatter.convertNumberToString(dateFormat, daysDiff);
    }

    private String getTimeString(String s)
    {
        if (s == null || s.equals(PropertyNames.EMPTY_STRING))
        {
            return PropertyNames.EMPTY_STRING;
        }
        int time = Integer.parseInt(s);

        double t = ((double) (time / 1000000) / 24) + ((double) ((time % 1000000) / 1000) / (24 * 60));
        return timeFormatter.convertNumberToString(timeFormat, t);
    }

    /* *******************************************
     *  F I N I S H
     *********************************************/
    /** the user clicked finish **/
    public synchronized void finish(List<PropertyValue[]> topics)
    {
        createMinutes(topics);
        deleteHiddenSections();
        textSectionHandler.removeAllTextSections();
    }

    /** 
     * hidden sections exist when an item's section is hidden because the
     * user specified not to display any items which it contains.
     * When finishing the wizard removes this sections entireley from the document.
     */
    private void deleteHiddenSections()
    {
        XTextSectionsSupplier xTextSectionsSupplier = UnoRuntime.queryInterface(XTextSectionsSupplier.class, document);
        String[] allSections = xTextSectionsSupplier.getTextSections().getElementNames();
        try
        {
            for (int i = 0; i < allSections.length; i++)
            {
                Object section = getSection(allSections[i]);
                //Try3.showProps(section);
                boolean visible = ((Boolean) Helper.getUnoPropertyValue(section, "IsVisible")).booleanValue();
                if (!visible)
                {
                    UnoRuntime.queryInterface(XTextContent.class, section).getAnchor().setString(PropertyNames.EMPTY_STRING);
                }
            }
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }

    /**
     * create the minutes for the given topics or remove the minutes section from the document.
     * If no topics are supplied, or the user 
     * specified not to create minuts, the minutes section will be removed, 
     * @param topicsData supplies PropertyValue arrays containing the values for the topics. 
     */
    public synchronized void createMinutes(List<PropertyValue[]> topicsData)
    {

        // if the minutes section should be removed (the
        // user did not check "create minutes")
        if (!agenda.cp_IncludeMinutes || (topicsData.size() <= 1))
        {
            try
            {
                Object minutesAllSection = getSection(SECTION_MINUTES_ALL);
                XTextSection xTextSection = UnoRuntime.queryInterface(XTextSection.class, minutesAllSection);
                xTextSection.getAnchor().setString(PropertyNames.EMPTY_STRING);
            }
            catch (Exception ex)
            {
                ex.printStackTrace();
            }
        }
        // the user checked "create minutes"
        else
        {
            try
            {
                String itemText;
                XTextRange item;
                int topicStartTime = 0;
                try
                {
                    topicStartTime = Integer.parseInt(agenda.cp_Time);
                }
                catch (Exception ex)
                {
                }

                String time;

                // first I replace the minutes titles...
                List<XTextRange> items = searchFillInItems();
                for (int itemIndex = 0; itemIndex < items.size(); itemIndex++)
                {
                    item = items.get(itemIndex);
                    itemText = item.getString().trim().toLowerCase();

                    if (itemText.equals(FILLIN_MINUTES_TITLE))
                    {
                        fillMinutesItem(item, agenda.cp_Title, resources.resPlaceHolderTitle);
                    }
                    else if (itemText.equals(FILLIN_MINUTES_LOCATION))
                    {
                        fillMinutesItem(item, agenda.cp_Location, resources.resPlaceHolderLocation);
                    }
                    else if (itemText.equals(FILLIN_MINUTES_DATE))
                    {
                        fillMinutesItem(item, getDateString(agenda.cp_Date), resources.resPlaceHolderDate);
                    }
                    else if (itemText.equals(FILLIN_MINUTES_TIME))
                    {
                        fillMinutesItem(item, getTimeString(agenda.cp_Time), resources.resPlaceHolderTime);
                    }
                }

                items.clear();

                /*
                 * now add minutes for each topic.
                 * The template contains *one* minutes section, so
                 * we first use the one available, and then add a new one...
                 * 
                 * topics data has *always* an empty topic at the end...
                 */
                for (int i = 0; i < topicsData.size() - 1; i++)
                {
                    PropertyValue[] topic = topicsData.get(i);

                    items = searchFillInItems();
                    for (int itemIndex = 0; itemIndex < items.size(); itemIndex++)
                    {
                        item = items.get(itemIndex);
                        itemText = item.getString().trim().toLowerCase();

                        if (itemText.equals(FILLIN_MINUTE_NUM))
                        {
                            fillMinutesItem(item, topic[0].Value, PropertyNames.EMPTY_STRING);
                        }
                        else if (itemText.equals(FILLIN_MINUTE_TOPIC))
                        {
                            fillMinutesItem(item, topic[1].Value, PropertyNames.EMPTY_STRING);
                        }
                        else if (itemText.equals(FILLIN_MINUTE_RESPONSIBLE))
                        {
                            fillMinutesItem(item, topic[2].Value, PropertyNames.EMPTY_STRING);
                        }
                        else if (itemText.equals(FILLIN_MINUTE_TIME))
                        {
                            int topicTime = 0;

                            try
                            {
                                topicTime = (new Integer((String) topic[3].Value)).intValue();
                            }
                            catch (Exception ex)
                            {
                            }
                            // if the topic has no time, we do not display any time here.
                            if (topicTime == 0 || topicStartTime == 0)
                            {
                                time = (String) topic[3].Value;
                            }
                            else
                            {
                                time = getTimeString(String.valueOf(topicStartTime)) + " - ";
                                topicStartTime += topicTime * 1000;
                                time += getTimeString(String.valueOf(topicStartTime));
                            }
                            fillMinutesItem(item, time, PropertyNames.EMPTY_STRING);
                        }
                    }

                    textSectionHandler.removeTextSectionbyName(SECTION_MINUTES);

                    // after the last section we do not insert a new one.
                    if (i < topicsData.size() - 2)
                    {
                        textSectionHandler.insertTextSection(SECTION_MINUTES, template, false);
                    }
                }
            }
            catch (Exception ex)
            {
                ex.printStackTrace();
            }
        }
    }

    /**
     * given a text range and a text, fills the given
     * text range with the given text.
     * If the given text is empty, uses a placeholder with the giveb placeholder text.
     * @param range text range to fill
     * @param text the text to fill to the text range object.
     * @param placeholder the placeholder text to use, if the text argument is empty (null or PropertyNames.EMPTY_STRING)
     */
    private void fillMinutesItem(XTextRange range, Object text, String placeholder)
    {
        String paraStyle = (String) Helper.getUnoPropertyValue(range, "ParaStyleName");
        range.setString((String) text);
        Helper.setUnoPropertyValue(range, "ParaStyleName", paraStyle);
        if (text == null || text.equals(PropertyNames.EMPTY_STRING))
        {
            if (placeholder != null && !placeholder.equals(PropertyNames.EMPTY_STRING))
            {
                XTextContent placeHolder = createPlaceHolder(docMSF, placeholder, resources.resPlaceHolderHint);
                try
                {
                    range.getStart().getText().insertTextContent(range.getStart(), placeHolder, true);
                }
                catch (Exception ex)
                {
                    ex.printStackTrace();
                }
            }

        }
    }

    /**
     * creates a placeholder field with the given text and given hint.
     * @param xmsf service factory
     * @param ph place holder text
     * @param hint hint text
     * @return the place holder field.
     */
    public static XTextContent createPlaceHolder(XMultiServiceFactory xmsf, String ph, String hint)
    {
        Object placeHolder;
        try
        {
            placeHolder = xmsf.createInstance("com.sun.star.text.TextField.JumpEdit");
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
            return null;
        }
        Helper.setUnoPropertyValue(placeHolder, "PlaceHolder", ph);
        Helper.setUnoPropertyValue(placeHolder, "Hint", hint);
        Helper.setUnoPropertyValue(placeHolder, "PlaceHolderType", new Short(PlaceholderType.TEXT));
        return UnoRuntime.queryInterface(XTextContent.class, placeHolder);

    }

    /*
     * $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
     * =================================
     *  The ItemTable class
     * =================================
     * $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
     */
    public class ItemsTable
    {

        Object table;
        Object section;
        /**
         * the items in the table.
         */
        List<AgendaItem> items = new ArrayList<AgendaItem>(6);

        public ItemsTable(Object section_, Object table_)
        {

            table = table_;
            section = section_;

            AgendaItem ai;
            XTextRange item;
            String iText;

            /* go through all <*> items in the document
             * and each one if it is in this table.
             * If they are, register them to belong here, notice their order
             * and remove them from the list of all <*> items, so the next
             * search will be faster.
             */
            for (int i = 0; i < _allItems.size(); i++)
            {
                item = _allItems.get(i);
                Object t = Helper.getUnoPropertyValue(item, "TextTable");
                if ((t instanceof Any) && ((Any) t).getObject() == table)
                {
                    iText = item.getString().toLowerCase().trim();
                    ai = itemsCache.get(item.getString().toLowerCase().trim());
                    if (ai != null)
                    {
                        items.add(ai);
                        _allItems.remove(i--);
                        itemsMap.put(iText, this);
                    }
                }
            }

        }

        /**
         * link the section to the template. this will restore the original table
         * with all the items.<br/>
         * then break the link, to make the section editable.<br/>
         * then, starting at cell one, write all items that should be visible.
         * then clear the rest and remove obsolete rows.
         * If no items are visible, hide the section.
         * @param dummy we need a param to make this an Implementation of AgendaElement.
         * @throws Exception
         */
        public synchronized void write(Object dummy) throws Exception
        {
            synchronized(this)
            {
                String name = getName(section);

                // link and unlink the section to the template.
                textSectionHandler.linkSectiontoTemplate(section, template, name);
                textSectionHandler.breakLinkOfTextSection(section);

                // we need to get a new instance after linking.
                table = getTable(name);
                section = getSection(name);

                XTextTable xTextTable = UnoRuntime.queryInterface(XTextTable.class, table);
                XTextTableCursor cursor = xTextTable.createCursorByCellName("A1");
                AgendaItem ai;
                // should this section be visible?
                boolean visible = false;

                // write items
                // ===========
                String cellName = PropertyNames.EMPTY_STRING;

                /* now go through all items that belong to this
                 * table. Check each one agains the model. If it should
                 * be display, call it's write method.
                 * All items are of type AgendaItem which means they write 
                 * two cells to the table: a title (text) and a placeholder.
                 * see AgendaItem class below. 
                 */
                for (int i = 0; i < items.size(); i++)
                {
                    ai = items.get(i);
                    if (isShowItem(ai.name))
                    {
                        visible = true;
                        ai.table = table;
                        ai.write(cursor);
                        // I store the cell name which was last written...
                        cellName = cursor.getRangeName();

                        cursor.goRight((short) 1, false);

                    }
                }

                Helper.setUnoPropertyValue(section, "IsVisible", visible ? Boolean.TRUE : Boolean.FALSE);
                if (!visible)
                {
                    return;
                /* remove obsolete rows
                 * ====================
                 * if the cell that was last written is the current cell,
                 * it means this is the end of the table, so we end here.
                 * (because after getting the cellName above, I call the goRight method.
                 * If it did not go right, it means its the last cell.
                 */
                }
                if (cellName.equals(cursor.getRangeName()))
                {
                    return;
                /*
                 * if not, we continue and clear all cells until we are at the end of the row.
                 */
                }
                Object cell;
                while ((!cellName.equals(cursor.getRangeName()) && (!cursor.getRangeName().startsWith("A"))))
                {
                    cell = xTextTable.getCellByName(cursor.getRangeName());
                    UnoRuntime.queryInterface(XTextRange.class, cell).setString(PropertyNames.EMPTY_STRING);
                    cellName = cursor.getRangeName();
                    cursor.goRight((short) 1, false);
                }

                /*
                 * again: if we are at the end of the table, end here.
                 */
                if (cellName.equals(cursor.getRangeName()))
                {
                    return;
                }
                int rowIndex = getRowIndex(cursor);
                int rowsCount = getRowCount(UnoRuntime.queryInterface(XTextTable.class, table));

                /* now before deleteing i move the cursor up so it 
                 * does not disappear, because it will crash office.
                 */
                cursor.gotoStart(false);

                if (rowsCount >= rowIndex)
                {
                    removeTableRows(table, rowIndex - 1, (rowsCount - rowIndex) + 1);
                }
            }
        }
    }

    /*
     * $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
     * =================================
     *  The Topics class
     * =================================
     * $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
     */
    /**
     * This class handles the preview of the topics table.
     * You can call it the controller of the topics table.
     * It differs from ItemsTable in that it has no data model - 
     * the update is done programttically.<br/>
     * <br/>
     * The decision to make this class a class by its own
     * was done out of logic reasons and not design/functionality reasons,
     * since there is anyway only one instance of this class at runtime
     * it could have also be implemented in the AgendaTemplate class
     * but for clarity and separation I decided to make a sub class for it.
     * 
     * @author rp143992
     */
    public class Topics
    {

        /**
         * the topics table
         */
        XTextTable table;
        /**
         * A List of Cell Formatters for the first row.
         */
        List<TableCellFormatter> firstRowFormat = new ArrayList<TableCellFormatter>();
        /**
         * A List of Cell Formatters for the last row.
         * (will contain them in reverse order)
         */
        List<TableCellFormatter> lastRowFormat = new ArrayList<TableCellFormatter>();
        /**
         * the format of the cell of each topic cell.
         */
        List<TableCellFormatter> topicCellFormats = new ArrayList<TableCellFormatter>();
        /**
         * for each topic cell there is
         * a member in this vector
         */
        List<AgendaElement> topicCells = new ArrayList<AgendaElement>();
        int rowsPerTopic;
        /**
         * fields which hold the number of the 
         * fillins in the cells vectors.
         */
        int numCell = -1;
        int topicCell = -1;
        int responsibleCell = -1;
        int timeCell = -1;
        /**
         * this is a list which traces which topics were written to the document
         * and which not. When a cell needs to be actualized, it is checked that the
         * whole topic is already present in the document, using this vector.
         * The vector contains nulls for topics which were not written, and
         * empty strings for topics which were written (though any other
         * object would also do - i check only if it is a null or not...);
         */
        List<String> writtenTopics = new ArrayList<String>();

        /**
         * Analyze the structure of the Topics table.
         * The structure Must be as follows:<br>
         * -One Header Row. <br> 
         * -arbitrary number of rows per topic <br>
         * -arbitrary content in the topics row <br>
         * -only soft formatting will be restored. <br>
         * -the topic rows must repeat three times. <br>
         * -in the topics rows, placeholders for number, topic, responsible, and duration
         * must be placed.<br>
         * <br>
         * A word about table format: to reconstruct the format of the
         * table we hold to the following formats: first row (header), topic, and last row.
         * We hold the format of the last row, because one might wish to give it
         * a special format, other than the one on the bottom of each topic.
         * The left and right borders of the whole table are, on the other side,
         * part of the topics rows format, and need not be preserved seperateley.
         */
        public Topics()
        {
            Object t;

            Map<String, AgendaElement> topicItems = new HashMap<String, AgendaElement>(4);

            // This is the topics table. say hallo :-)
            try
            {
                t = getTable(SECTION_TOPICS);
            }
            catch (Exception ex)
            {
                ex.printStackTrace();
                throw new IllegalArgumentException("Fatal error while loading template: table " + SECTION_TOPICS + " could not load.");
            }

            // and this is the XTable.
            table = UnoRuntime.queryInterface(XTextTable.class, t);

            /* first I store all <*> ranges
             * which are in the topics table.
             * I store each <*> range in this - the key
             * is the cell it is in. Later when analyzing the topic,
             * cell by cell, I check in this map to know
             * if a cell contains a <*> or not.
             */
            HashMap<Object, XTextRange> items = new HashMap<Object, XTextRange>();

            XTextRange item;
            Object cell;
            for (int i = 0; i < _allItems.size(); i++)
            {
                item = _allItems.get(i);
                t = Helper.getUnoPropertyValue(item, "TextTable");
                if ((t instanceof Any) && ((Any) t).getObject() == table)
                {
                    cell = Helper.getUnoPropertyValue(item, "Cell");
                    items.put(((Any) cell).getObject(), item);
                }
            }

            /*
             * in the topics table, there are always one 
             * title row and three topics defined.
             * So no mutter how many rows a topic takes - we
             * can restore its structure and format.
             */
            int rows = getRowCount(table);

            rowsPerTopic = (rows - 1) / 3;

            String firstCell = "A" + (1 + rowsPerTopic + 1);
            String afterLastCell = "A" + (1 + (rowsPerTopic * 2) + 1);

            // go to the first row of the 2. topic
            XTextTableCursor cursor = table.createCursorByCellName(firstCell);
            XTextRange range;

            // analyze the structure of the topic rows.
            while (!cursor.getRangeName().equals(afterLastCell))
            {
                cell = table.getCellByName(cursor.getRangeName());
                XTextRange xTextRange = UnoRuntime.queryInterface(XTextRange.class, cell);
                // first I store the content and para style of the cell
                AgendaElement ae = new TextElement(xTextRange);
                // if the cell contains a relevant <...>
                // i add the text element to the hash,
                // so it's text can be updated later.
                range = items.get(cell);
                if (range != null)
                {
                    topicItems.put(xTextRange.getString().toLowerCase().trim(), ae);
                }

                topicCells.add(ae);

                // and store the format of the cell.
                topicCellFormats.add(new TableCellFormatter(table.getCellByName(cursor.getRangeName())));

                // goto next cell.
                cursor.goRight((short) 1, false);
            }

            /*
             * now - in which cell is every fillin?
             */
            numCell = topicCells.indexOf(topicItems.get(FILLIN_TOPIC_NUMBER));
            topicCell = topicCells.indexOf(topicItems.get(FILLIN_TOPIC_TOPIC));
            responsibleCell = topicCells.indexOf(topicItems.get(FILLIN_TOPIC_RESPONSIBLE));
            timeCell = topicCells.indexOf(topicItems.get(FILLIN_TOPIC_TIME));



            /* now that we know how the topics look like, 
             * we get the format of the first and last rows.
             */

            // format of first row
            cursor.gotoStart(false);
            do
            {
                firstRowFormat.add(new TableCellFormatter(table.getCellByName(cursor.getRangeName())));
                cursor.goRight((short) 1, false);
            }
            while (!cursor.getRangeName().startsWith("A"));

            // format of the last row
            cursor.gotoEnd(false);
            while (!cursor.getRangeName().startsWith("A"))
            {
                lastRowFormat.add(new TableCellFormatter(table.getCellByName(cursor.getRangeName())));
                cursor.goLeft((short) 1, false);
            }
            // we missed the A cell - so we have to add it also..
            lastRowFormat.add(new TableCellFormatter(table.getCellByName(cursor.getRangeName())));

            removeTableRows(table, 1 + rowsPerTopic, rows - rowsPerTopic - 1);

        }

        /**
         * @param topic the topic number to write
         * @param data the data of the topic.
         * @return the number of rows that have been added
         * to the table. 0 or a negative number: no rows added.
         */
        private int write2(int topic, PropertyValue[] data) throws Exception
        {
            while (topic >= writtenTopics.size())
            {
                writtenTopics.add(null);
            }
            writtenTopics.set(topic, PropertyNames.EMPTY_STRING);

            // make sure threr are enough rows for me...
            int rows = getRowCount(table);
            int reqRows = 1 + (topic + 1) * rowsPerTopic;
            int firstRow = reqRows - rowsPerTopic + 1;
            int diff = reqRows - rows;
            if (diff > 0)
            {
                insertTableRows(table, rows, diff);            // set the item's text...
            }
            setItemText(numCell, data[0].Value);
            setItemText(topicCell, data[1].Value);
            setItemText(responsibleCell, data[2].Value);
            setItemText(timeCell, data[3].Value);

            // now write !
            XTextTableCursor cursor = table.createCursorByCellName("A" + firstRow);

            for (int i = 0; i < topicCells.size(); i++)
            {
                topicCells.get(i).write(table.getCellByName(cursor.getRangeName()));
                cursor.goRight((short) 1, false);
            }

            // now format !
            cursor.gotoCellByName("A" + firstRow, false);

            formatTable(cursor, topicCellFormats, false);

            return diff;

        }

        /**
         * check if the topic with the given index is written to the table.
         * @param topic the topic number (0 base)
         * @return true if the topic is already written to the table. False if not. 
         * (false would mean new rows must be added to the table in order to 
         * be able to write this topic).
         */
        private boolean isWritten(int topic)
        {
            return (writtenTopics.size() > topic && writtenTopics.get(topic) != null);
        }

        /**
         * rewrites a single cell containing.
         * This is used in order to refresh the topic/responsible/duration data in the 
         * preview document, in response to a change in the gui (by the user).
         * Since the structure of the topics table is flexible, we don't reference a cell
         * number. Rather, we use "what" argument to specify which cell should be redrawn.
         * The Topics object, which analyzed the structure of the topics table appon 
         * initialization, refreshes the approperiate cell.
         * @param topic index of the topic (0 based).
         * @param what 0 for num, 1 for topic, 2 for responsible, 3 for duration
         * @param data the row's data.
         * @throws Exception if something goes wrong (thow nothing should)
         */
        public void writeCell(int topic, int what, PropertyValue[] data) throws Exception
        {
            // if the whole row should be written...
            if (!isWritten(topic))
            {
                write(topic, data);
            // write only the "what" cell.
            }
            else
            {
                // calculate the table row.
                int firstRow = 1 + (topic * rowsPerTopic) + 1;
                // go to the first cell of this topic.
                XTextTableCursor cursor = table.createCursorByCellName("A" + firstRow);

                TextElement te = null;
                int cursorMoves = 0;

                switch (what)
                {
                    case 0:
                        te = setItemText(numCell, data[0].Value);
                        cursorMoves = numCell;
                        break;
                    case 1:
                        te = setItemText(topicCell, data[1].Value);
                        cursorMoves = topicCell;
                        break;
                    case 2:
                        te = setItemText(responsibleCell, data[2].Value);
                        cursorMoves = responsibleCell;
                        break;
                    case 3:
                        te = setItemText(timeCell, data[3].Value);
                        cursorMoves = timeCell;
                        break;
                }
                // move the cursor to the needed cell...
                if ( te != null)
                {
                    cursor.goRight((short) cursorMoves, false);
                    XCell xc = table.getCellByName(cursor.getRangeName());
                    // and write it !
                    te.write(xc);
                    topicCellFormats.get(cursorMoves).format(xc);
                }
            }
        }

        /**
         * writes the given topic.
         * if the first topic was involved, reformat the
         * first row.
         * If any rows were added to the table, reformat 
         * the last row. 
         * @param topic the index of the topic to write.
         * @param data the topic's data. (see TopicsControl 
         * for explanation about the topics data model)
         * @throws Exception if something goes wrong (though nothing should).
         */
        public void write(int topic, PropertyValue[] data) throws Exception
        {
            int diff = write2(topic, data);
            /* if the first topic has been written,
             * one needs to reformat the first row.
             */
            if (topic == 0)
            {
                formatFirstRow();
            }
            /*
             * if any rows were added, one needs to format
             * the whole table again.
             */
            if (diff > 0)
            {
                formatLastRow();
            }
        }

        /**
         * Writes all the topics to thetopics table.
         * @param topicsData a List containing all Topic's Data.
         */
        public void writeAll(List<PropertyValue[]> topicsData)
        {
            try
            {
                for (int i = 0; i < topicsData.size() - 1; i++)
                {
                    write2(i, topicsData.get(i));
                }
                formatLastRow();
            }
            catch (Exception ex)
            {
                ex.printStackTrace();
            }
        }

        /**
         * removes obsolete rows, reducing the
         * topics table to the given number of topics.
         * Note this method does only reducing - if
         * the number of topics given is greater than the
         * number of actuall topics it does *not* add
         * new rows !
         * Note also that the first topic will never be removed.
         * If the table contains no topics, the whole section will 
         * be removed uppon finishing.
         * The reason for that is a "table-design" one: the first topic is
         * maintained in order to be able to add rows with a design of this topic, 
         * and not of the header row.
         * @param topics the number of topics the table should contain.
         * @throws Exception
         */
        public void reduceDocumentTo(int topics) throws Exception
        {
            // we never remove the first topic...
            if (topics <= 0)
            {
                topics = 1;
            }
            XTableRows tableRows = table.getRows();
            int targetNumOfRows = topics * rowsPerTopic + 1;
            if (tableRows.getCount() > targetNumOfRows)
            {
                //tableRows.removeByIndex(targetNumOfRows, tableRows.getCount() - targetNumOfRows);
            }
            formatLastRow();
            while (writtenTopics.size() > topics)
            {
                writtenTopics.remove(topics);
            }
        }

        /**
         * reapply the format of the first (header) row.
         */
        private void formatFirstRow()
        {
            XTextTableCursor cursor = table.createCursorByCellName("A1");
            formatTable(cursor, firstRowFormat, false);
        }

        /**
         * reaply the format of the last row.
         */
        private void formatLastRow()
        {
            XTextTableCursor cursor = table.createCursorByCellName("A1");
            cursor.gotoEnd(false);
            formatTable(cursor, lastRowFormat, true);
        }

        /**
         * returns a text element for the given cell,
         * which will write the given text.
         * @param cell the topics cell number. 
         * @param value the value to write.
         * @return a TextElement object which will write the given value
         * to the given cell.
         */
        private TextElement setItemText(int cell, Object value)
        {
            if (cell >= 0)
            {
                TextElement te = ((TextElement) topicCells.get(cell));
                if (te != null)
                {
                    te.text = value.toString();
                }
                return te;
            }
            return null;
        }

        /**
         * formats a series of cells from the given one,
         * using the given List of TableCellFormatter objects,
         * in the given order.
         * This method is used to format the first (header) and the last 
         * rows of the table.
         * @param cursor a table cursor, pointing to the start cell to format
         * @param formats a List containing TableCellFormatter objects. Each will format one cell in the direction specified.
         * @param reverse if true the cursor will move left, formatting in reverse order (used for the last row).
         */
        private void formatTable(XTextTableCursor cursor, List<TableCellFormatter> formats, boolean reverse)
        {
            for (int i = 0; i < formats.size(); i++)
            {
                formats.get(i).format(table.getCellByName(cursor.getRangeName()));
                if (reverse)
                {
                    cursor.goLeft((short) 1, false);
                }
                else
                {
                    cursor.goRight((short) 1, false);
                }
            }
        }
    }


    /*
     * =================================
     * Here are some static help methods
     * =================================
     */
    public static String[] getNamesWhichStartWith(String[] allNames, String prefix)
    {
        ArrayList<String> v = new ArrayList<String>();
        for (int i = 0; i < allNames.length; i++)
        {
            if (allNames[i].startsWith(prefix))
            {
                v.add(allNames[i]);
            }
        }
        String[] s = new String[v.size()];
        return v.toArray(s);
    }

    /**
     * Convenience method, costs the given object to an XNamed, and returnes its name.
     * @param obj an XNamed object.
     * @return the name of the given object.
     */
    public static String getName(Object obj)
    {
        return UnoRuntime.queryInterface(XNamed.class, obj).getName();
    }

    /**
     * convenience method, for removing a number of cells from a table.
     * @param table
     * @param start
     * @param count
     */
    public static void removeTableRows(Object table, int start, int count)
    {
        XTableRows rows = UnoRuntime.queryInterface(XTextTable.class, table).getRows();
        //rows.removeByIndex(start, count);
    }

    /**
     * Convenience method for inserting some cells into a table.
     * @param table
     * @param start
     * @param count
     */
    public static void insertTableRows(Object table, int start, int count)
    {
        XTableRows rows = UnoRuntime.queryInterface(XTextTable.class, table).getRows();
        rows.insertByIndex(start, count);
    }

    /**
     * returns the row index for this cursor, assuming
     * the cursor points to a single cell.
     * @param cursor
     * @return the row index in which the cursor is.
     */
    public static int getRowIndex(XTextTableCursor cursor)
    {
        return getRowIndex(cursor.getRangeName());
    }

    /**
     * returns the row index for this cell name.
     * @param cellName
     * @return the row index for this cell name.
     */
    public static int getRowIndex(String cellName)
    {
        return Integer.parseInt(cellName.substring(1));
    }

    /**
     * returns the rows count of this table, assuming 
     * there is no vertical merged cells.
     * @param table
     * @return the rows count of the given table.
     */
    public static int getRowCount(XTextTable table)
    {
        String[] cells = table.getCellNames();
        return getRowIndex(cells[cells.length - 1]);
    }
}

/*
 * ===========================================================================================
 * 
 *                  End of AgendaTempalte class
 *  
 * ===========================================================================================
 * 
 */
/*
 * =================================
 *  The AgendaElement interface
 * =================================
 */
/**
 * Interface that is used for writing content to a Uno Text / TextRange 
 * @author rp143992
 *
 */
interface AgendaElement
{

    void write(Object any) throws Exception;
}


/*
 * =================================
 *  The ParaStyled class
 * =================================
 */
/**
 * Basic implementation of the AgendaElement interface - 
 * writes nothing, but applies a ParaStyle to the given XText/XTextRange 
 * @author rp143992
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
class ParaStyled implements AgendaElement
{

    String paraStyle;

    ParaStyled(String paraStyle_)
    {
        paraStyle = paraStyle_;
    }

    void format(Object textRange)
    {
        XText o;
        o = UnoRuntime.queryInterface(XText.class, textRange);
        if (o == null)
        {
            o = UnoRuntime.queryInterface(XTextRange.class, textRange).getText();
        }
        XTextRange xtr = UnoRuntime.queryInterface(XTextRange.class, textRange);
        XTextCursor cursor = o.createTextCursorByRange(xtr);

        Helper.setUnoPropertyValue(cursor, "ParaStyleName", paraStyle);
    }

    public void write(Object textRange)
    {
        format(textRange);
    }
}

/*
 * =================================
 *  The TextElement class
 * =================================
 */
/**
 * A basic implementation of AgendaElement:
 * writes a String to the given XText/XTextRange, and applies
 * a ParaStyle to it (using the parent class).
 * @author rp143992
 */
class TextElement extends ParaStyled
{

    String text;

    TextElement(XTextRange range)
    {
        this(range.getString(), (String) Helper.getUnoPropertyValue(range.getStart(), "ParaStyleName"));
    }

    TextElement(String text_, String paraStyle_)
    {
        super(paraStyle_);
        text = text_;
    }

    public void write(Object textRange)
    {
        UnoRuntime.queryInterface(XTextRange.class, textRange).setString(text);
        if (!text.equals(PropertyNames.EMPTY_STRING))
        {
            super.write(textRange);
        }
    }
}

/**
 * A Text element which, if the text to write is empty (null or PropertyNames.EMPTY_STRING)
 * inserts a placeholder instead.
 * @author rp143992
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
class PlaceholderTextElement extends TextElement
{

    String hint;
    String placeHolderText;
    XMultiServiceFactory xmsf;

    PlaceholderTextElement(XTextRange textRange, String placeHolderText_, String hint_, XMultiServiceFactory xmsf_)
    {
        super(textRange);
        placeHolderText = placeHolderText_;
        hint = hint_;
        xmsf = xmsf_;
    }

    PlaceholderTextElement(String text, String paraStyle, String placeHolderText_, String hint_, XMultiServiceFactory xmsf_)
    {
        super(text, paraStyle);
        placeHolderText = placeHolderText_;
        hint = hint_;
        xmsf = xmsf_;
    }

    public void write(Object textRange)
    {
        super.write(textRange);
        if (text == null || text.equals(PropertyNames.EMPTY_STRING))
        {
            XTextRange xTextRange = UnoRuntime.queryInterface(XTextRange.class, textRange);
            try
            {
                XTextContent xTextContent = AgendaTemplate.createPlaceHolder(xmsf, placeHolderText, hint);
                xTextRange.getText().insertTextContent(xTextRange.getStart(), xTextContent, true);
            }
            catch (Exception ex)
            {
                ex.printStackTrace();
            }
        }
    }
}

/*
 * =================================
 *  The PlaceHolder class
 * =================================
 */
/** 
 * An Agenda element which writes no text, but inserts a placeholder, and formats
 * it using a ParaStyleName.
 * @author rp143992
 *
 */
class PlaceholderElement extends ParaStyled
{

    String hint;
    String placeHolderText;
    XMultiServiceFactory xmsf;

    PlaceholderElement(String paraStyle, String placeHolderText_, String hint_, XMultiServiceFactory xmsf_)
    {
        super(paraStyle);
        placeHolderText = placeHolderText_;
        hint = hint_;
        xmsf = xmsf_;
    }

    public void write(Object textRange)
    {
        XTextRange xTextRange = UnoRuntime.queryInterface(XTextRange.class, textRange);
        try
        {
            XTextContent xTextContent = AgendaTemplate.createPlaceHolder(xmsf, placeHolderText, hint);
            xTextRange.getText().insertTextContent(xTextRange.getStart(), xTextContent, true);
            super.write(textRange);
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }
}


/*
 * =================================
 *  The AgendaItem class
 * =================================
 */
/**
 * An implementation of AgendaElement which 
 * gets as a parameter a table cursor, and writes
 * a text to the cell marked by this table cursor, and
 * a place holder to the next cell.
 * @author rp143992
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
class AgendaItem implements AgendaElement
{

    TextElement textElement;
    AgendaElement field;
    public Object table;
    String name;

    AgendaItem(String name_, TextElement te, AgendaElement f)
    {
        name = name_;
        field = f;
        textElement = te;
    }

    public void write(Object tableCursor) throws Exception
    {
        XTextTableCursor xTextTableCursor = UnoRuntime.queryInterface(XTextTableCursor.class, tableCursor);
        XTextTable xTextTable = UnoRuntime.queryInterface(XTextTable.class, table);

        String cellname = xTextTableCursor.getRangeName();
        Object cell = xTextTable.getCellByName(cellname);

        textElement.write(cell);

        xTextTableCursor.goRight((short) 1, false);

        //second field is actually always null...
        // this is a preparation for adding placeholders.
        if (field != null)
        {
            field.write(xTextTable.getCellByName(xTextTableCursor.getRangeName()));
        }
    }
}

/*
 * =================================
 *  The TableCellFormatter class
 * =================================
 */
/**
 * reads/write a table cell format from/to a table cell or a group of cells.
 *
 */
class TableCellFormatter
{

    static String[] properties = new String[]
    {
        "BackColor",
        "BackTransparent",
        "BorderDistance",
        "BottomBorder",
        "BottomBorderDistance",
        "LeftBorder",
        "LeftBorderDistance",
        "RightBorder",
        "RightBorderDistance",
        "TopBorder",
        "TopBorderDistance"
    };
    private Object[] values = new Object[properties.length];

    public TableCellFormatter(Object tableCell)
    {
        for (int i = 0; i < properties.length; i++)
        {
            values[i] = Helper.getUnoPropertyValue(tableCell, properties[i]);
        }
    }

    public void format(Object tableCell)
    {
        Helper.setUnoPropertyValues(tableCell, properties, values);
    }
}

    


