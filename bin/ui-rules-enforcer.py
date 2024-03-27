#!/bin/python
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# ui-rules-enforcer enforces the .ui rules and properties used by LibreOffice
# mostly the deprecations of
# https://developer.gnome.org/gtk4/stable/gtk-migrating-3-to-4.html
# and a few other home cooked rules

# for any existing .ui this should parse it and overwrite it with the same content
# e.g. for a in `git ls-files "*.ui"`; do bin/ui-rules-enforcer.py $a; done

import lxml.etree as etree
import sys

def add_truncate_multiline(current):
  use_truncate_multiline = False
  istarget = current.get('class') == "GtkEntry" or current.get('class') == "GtkSpinButton"
  insertpos = 0
  for child in current:
    add_truncate_multiline(child)
    insertpos = insertpos + 1
    if not istarget:
        continue
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "truncate_multiline" or attributes.get("name") == "truncate-multiline":
        use_truncate_multiline = True

  if istarget and not use_truncate_multiline:
      truncate_multiline = etree.Element("property")
      attributes = truncate_multiline.attrib
      attributes["name"] = "truncate-multiline"
      truncate_multiline.text = "True"
      current.insert(insertpos - 1, truncate_multiline)

def do_replace_button_use_stock(current, use_stock, use_underline, label, insertpos):
  if not use_underline:
      underline = etree.Element("property")
      attributes = underline.attrib
      attributes["name"] = "use-underline"
      underline.text = "True"
      current.insert(insertpos - 1, underline)
  current.remove(use_stock)
  attributes = label.attrib
  attributes["translatable"] = "yes"
  attributes["context"] = "stock"
  if label.text == 'gtk-add':
    label.text = "_Add"
  elif label.text == 'gtk-apply':
    label.text = "_Apply"
  elif label.text == 'gtk-cancel':
    label.text = "_Cancel"
  elif label.text == 'gtk-close':
    label.text = "_Close"
  elif label.text == 'gtk-delete':
    label.text = "_Delete"
  elif label.text == 'gtk-edit':
    label.text = "_Edit"
  elif label.text == 'gtk-help':
    label.text = "_Help"
  elif label.text == 'gtk-new':
    label.text = "_New"
  elif label.text == 'gtk-no':
    label.text = "_No"
  elif label.text == 'gtk-ok':
    label.text = "_OK"
  elif label.text == 'gtk-remove':
    label.text = "_Remove"
  elif label.text == 'gtk-revert-to-saved':
    label.text = "_Reset"
  elif label.text == 'gtk-yes':
    label.text = "_Yes"
  else:
    raise Exception(sys.argv[1] + ': unknown label', label.text)

def replace_button_use_stock(current):
  use_underline = False
  use_stock = None
  label = None
  isbutton = current.get('class') == "GtkButton"
  insertpos = 0
  for child in current:
    replace_button_use_stock(child)
    insertpos = insertpos + 1
    if not isbutton:
        continue
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "use_underline" or attributes.get("name") == "use-underline":
        use_underline = True
      if attributes.get("name") == "use_stock" or attributes.get("name") == "use-stock":
        use_stock = child
      if attributes.get("name") == "label":
        label = child

  if isbutton and use_stock != None:
    do_replace_button_use_stock(current, use_stock, use_underline, label, insertpos)

def do_replace_image_stock(current, stock):
  attributes = stock.attrib
  attributes["name"] = "icon-name"
  if stock.text == 'gtk-add':
    stock.text = "list-add"
  elif stock.text == 'gtk-remove':
    stock.text = "list-remove"
  elif stock.text == 'gtk-paste':
    stock.text = "edit-paste"
  elif stock.text == 'gtk-index':
    stock.text = "vcl/res/index.png"
  elif stock.text == 'gtk-refresh':
    stock.text = "view-refresh"
  elif stock.text == 'gtk-dialog-error':
    stock.text = "dialog-error"
  elif stock.text == 'gtk-apply':
    stock.text = "sw/res/sc20558.png"
  elif stock.text == 'gtk-missing-image':
    stock.text = "missing-image"
  elif stock.text == 'gtk-copy':
    stock.text = "edit-copy"
  elif stock.text == 'gtk-go-back':
    stock.text = "go-previous"
  elif stock.text == 'gtk-go-forward':
    stock.text = "go-next"
  elif stock.text == 'gtk-go-down':
    stock.text = "go-down"
  elif stock.text == 'gtk-go-up':
    stock.text = "go-up"
  elif stock.text == 'gtk-goto-first':
    stock.text = "go-first"
  elif stock.text == 'gtk-goto-last':
    stock.text = "go-last"
  elif stock.text == 'gtk-new':
    stock.text = "document-new"
  elif stock.text == 'gtk-open':
    stock.text = "document-open"
  elif stock.text == 'gtk-media-stop':
    stock.text = "media-playback-stop"
  elif stock.text == 'gtk-media-play':
    stock.text = "media-playback-start"
  elif stock.text == 'gtk-media-next':
    stock.text = "media-skip-forward"
  elif stock.text == 'gtk-media-previous':
    stock.text = "media-skip-backward"
  elif stock.text == 'gtk-close':
    stock.text = "window-close"
  elif stock.text == 'gtk-help':
    stock.text = "help-browser"
  else:
    raise Exception(sys.argv[1] + ': unknown stock name', stock.text)

def replace_image_stock(current):
  stock = None
  isimage = current.get('class') == "GtkImage"
  for child in current:
    replace_image_stock(child)
    if not isimage:
        continue
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "stock":
        stock = child

  if isimage and stock != None:
    do_replace_image_stock(current, stock)

def remove_check_button_align(current):
  xalign = None
  yalign = None
  ischeckorradiobutton = current.get('class') == "GtkCheckButton" or current.get('class') == "GtkRadioButton"
  for child in current:
    remove_check_button_align(child)
    if not ischeckorradiobutton:
        continue
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "xalign":
        xalign = child
      if attributes.get("name") == "yalign":
        yalign = child

  if ischeckorradiobutton:
    if xalign != None:
      if xalign.text != "0":
        raise Exception(sys.argv[1] + ': non-default xalign', xalign.text)
      current.remove(xalign)
    if yalign != None:
      if yalign.text != "0.5":
        raise Exception(sys.argv[1] + ': non-default yalign', yalign.text)
      current.remove(yalign)

def remove_check_button_relief(current):
  relief = None
  ischeckorradiobutton = current.get('class') == "GtkCheckButton" or current.get('class') == "GtkRadioButton"
  for child in current:
    remove_check_button_relief(child)
    if not ischeckorradiobutton:
        continue
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "relief":
        relief = child

  if ischeckorradiobutton:
    if relief != None:
      current.remove(relief)

def remove_check_button_image_position(current):
  image_position = None
  ischeckorradiobutton = current.get('class') == "GtkCheckButton" or current.get('class') == "GtkRadioButton"
  for child in current:
    remove_check_button_image_position(child)
    if not ischeckorradiobutton:
        continue
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "image_position" or attributes.get("name") == "image-position":
        image_position = child

  if ischeckorradiobutton:
    if image_position != None:
      current.remove(image_position)

def remove_spin_button_input_purpose(current):
  input_purpose = None
  isspinbutton = current.get('class') == "GtkSpinButton"
  for child in current:
    remove_spin_button_input_purpose(child)
    if not isspinbutton:
        continue
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "input_purpose" or attributes.get("name") == "input-purpose":
        input_purpose = child

  if isspinbutton:
    if input_purpose != None:
      current.remove(input_purpose)

def remove_caps_lock_warning(current):
  caps_lock_warning = None
  iscandidate = current.get('class') == "GtkSpinButton" or current.get('class') == "GtkEntry"
  for child in current:
    remove_caps_lock_warning(child)
    if not iscandidate:
        continue
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "caps_lock_warning" or attributes.get("name") == "caps-lock-warning":
        caps_lock_warning = child

  if iscandidate:
    if caps_lock_warning != None:
      current.remove(caps_lock_warning)

def remove_spin_button_max_length(current):
  max_length = None
  isspinbutton = current.get('class') == "GtkSpinButton"
  for child in current:
    remove_spin_button_max_length(child)
    if not isspinbutton:
        continue
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "max_length" or attributes.get("name") == "max-length":
        max_length = child

  if isspinbutton:
    if max_length != None:
      current.remove(max_length)

def remove_entry_shadow_type(current):
  shadow_type = None
  isentry = current.get('class') == "GtkEntry"
  for child in current:
    remove_entry_shadow_type(child)
    if not isentry:
        continue
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "shadow_type" or attributes.get("name") == "shadow-type":
        shadow_type = child

  if isentry:
    if shadow_type!= None:
      current.remove(shadow_type)

def remove_label_pad(current):
  xpad = None
  ypad = None
  islabel = current.get('class') == "GtkLabel"
  for child in current:
    remove_label_pad(child)
    if not islabel:
        continue
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "xpad":
        xpad = child
      elif attributes.get("name") == "ypad":
        ypad = child

  if xpad != None:
    current.remove(xpad)
  if ypad != None:
    current.remove(ypad)

def remove_label_angle(current):
  angle = None
  islabel = current.get('class') == "GtkLabel"
  for child in current:
    remove_label_angle(child)
    if not islabel:
        continue
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "angle":
        angle = child

  if angle != None:
    current.remove(angle)

def remove_track_visited_links(current):
  track_visited_links = None
  islabel = current.get('class') == "GtkLabel"
  for child in current:
    remove_track_visited_links(child)
    if not islabel:
        continue
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "track_visited_links" or attributes.get("name") == "track-visited-links":
        track_visited_links = child

  if track_visited_links != None:
    current.remove(track_visited_links)

def remove_toolbutton_focus(current):
  can_focus = None
  classname = current.get('class')
  istoolbutton = classname and classname.endswith("ToolButton")
  for child in current:
    remove_toolbutton_focus(child)
    if not istoolbutton:
        continue
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "can_focus" or attributes.get("name") == "can-focus":
        can_focus = child

  if can_focus != None:
    current.remove(can_focus)

def remove_double_buffered(current):
  double_buffered = None
  for child in current:
    remove_double_buffered(child)
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "double_buffered" or attributes.get("name") == "double-buffered":
        double_buffered = child

  if double_buffered != None:
    current.remove(double_buffered)

def remove_label_yalign(current):
  label_yalign = None
  for child in current:
    remove_label_yalign(child)
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "label_yalign" or attributes.get("name") == "label-yalign":
        label_yalign = child

  if label_yalign != None:
    current.remove(label_yalign)

def remove_skip_pager_hint(current):
  skip_pager_hint = None
  for child in current:
    remove_skip_pager_hint(child)
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "skip_pager_hint" or attributes.get("name") == "skip-pager-hint":
        skip_pager_hint = child

  if skip_pager_hint != None:
    current.remove(skip_pager_hint)

def remove_gravity(current):
  gravity = None
  for child in current:
    remove_gravity(child)
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "gravity":
        gravity = child

  if gravity != None:
    current.remove(gravity)

def remove_expander_label_fill(current):
  label_fill = None
  isexpander = current.get('class') == "GtkExpander"
  for child in current:
    remove_expander_label_fill(child)
    if not isexpander:
        continue
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "label_fill" or attributes.get("name") == "label-fill":
        label_fill = child

  if label_fill != None:
    current.remove(label_fill)

def remove_expander_spacing(current):
  spacing = None
  isexpander = current.get('class') == "GtkExpander"
  for child in current:
    remove_expander_spacing(child)
    if not isexpander:
        continue
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "spacing":
        spacing = child

  if spacing != None:
    current.remove(spacing)

def enforce_menubutton_indicator_consistency(current):
  draw_indicator = None
  image = None
  ismenubutton = current.get('class') == "GtkMenuButton"
  insertpos = 0
  for child in current:
    enforce_menubutton_indicator_consistency(child)
    if not ismenubutton:
      continue
    if child.tag == "property":
      insertpos = insertpos + 1
      attributes = child.attrib
      if attributes.get("name") == "draw_indicator" or attributes.get("name") == "draw-indicator":
        draw_indicator = child
      elif attributes.get("name") == "image":
        image = child

  if ismenubutton:
    if draw_indicator == None:
      if image == None:
        # if there is no draw indicator and no image there should be a draw indicator
        draw_indicator = etree.Element("property")
        attributes = draw_indicator.attrib
        attributes["name"] = "draw-indicator"
        draw_indicator.text = "True"
        current.insert(insertpos, draw_indicator)
      else:
        # if there is no draw indicator but there is an image that image should be open-menu-symbolic or x-office-calendar
        for status_elem in tree.xpath("/interface/object[@id='" + image.text + "']/property[@name='icon_name' or @name='icon-name']"):
          if status_elem.text != 'x-office-calendar':
            status_elem.text = "open-menu-symbolic"

def enforce_active_in_group_consistency(current):
  group = None
  active = None
  isradiobutton = current.get('class') == "GtkRadioButton"
  insertpos = 0
  for child in current:
    enforce_active_in_group_consistency(child)
    if not isradiobutton:
        continue
    if child.tag == "property":
      insertpos = insertpos + 1
      attributes = child.attrib
      if attributes.get("name") == "group":
        group = child
      if attributes.get("name") == "active":
        active = child

  if isradiobutton:
    if active != None and active.text != "True":
      raise Exception(sys.argv[1] + ': non-standard active value', active.text)
    if group != None and active != None:
      # if there is a group then we are not the leader and should not be active
      current.remove(active)
    elif group == None and active == None:
      # if there is no group then we are the leader and should be active
      active = etree.Element("property")
      attributes = active.attrib
      attributes["name"] = "active"
      active.text = "True"
      current.insert(insertpos, active)

def enforce_toolbar_can_focus(current):
  can_focus = None
  istoolbar = current.get('class') == "GtkToolbar"
  insertpos = 0
  for child in current:
    enforce_toolbar_can_focus(child)
    if not istoolbar:
        continue
    if child.tag == "property":
      insertpos = insertpos + 1
      attributes = child.attrib
      if attributes.get("name") == "can-focus" or attributes.get("name") == "can_focus":
        can_focus = child

  if istoolbar:
    if can_focus == None:
      can_focus = etree.Element("property")
      attributes = can_focus.attrib
      attributes["name"] = "can-focus"
      can_focus.text = "True"
      current.insert(insertpos, can_focus)
    else:
      can_focus.text = "True"

def enforce_entry_text_column_id_column_for_gtkcombobox(current):
  entrytextcolumn = None
  idcolumn = None
  isgtkcombobox = current.get('class') == "GtkComboBox"
  insertpos = 0
  for child in current:
    enforce_entry_text_column_id_column_for_gtkcombobox(child)
    if not isgtkcombobox:
        continue
    if child.tag == "property":
      insertpos = insertpos + 1
      attributes = child.attrib
      if attributes.get("name") == "entry_text_column" or attributes.get("name") == "entry-text-column":
        entrytextcolumn = child
      if attributes.get("name") == "id_column" or attributes.get("name") == "id-column":
        idcolumn = child

  if isgtkcombobox:
    if entrytextcolumn != None and entrytextcolumn.text != "0":
      raise Exception(sys.argv[1] + ': non-standard entry_text_column value', entrytextcolumn.text)
    if idcolumn != None and idcolumn.text != "1":
      raise Exception(sys.argv[1] + ': non-standard id_column value', idcolumn.text)
    if entrytextcolumn == None:
      # if there is no entry_text_column, create one
      entrytextcolumn = etree.Element("property")
      attributes = entrytextcolumn.attrib
      attributes["name"] = "entry-text-column"
      entrytextcolumn.text = "0"
      current.insert(insertpos, entrytextcolumn)
      insertpos = insertpos + 1
    if idcolumn == None:
      # if there is no id_column, create one
      idcolumn = etree.Element("property")
      attributes = idcolumn.attrib
      attributes["name"] = "id-column"
      idcolumn.text = "1"
      current.insert(insertpos, idcolumn)

def enforce_button_always_show_image(current):
  image = None
  always_show_image = None
  isbutton = current.get('class') == "GtkButton"
  insertpos = 0
  for child in current:
    enforce_button_always_show_image(child)
    if not isbutton:
        continue
    if child.tag == "property":
      insertpos = insertpos + 1
      attributes = child.attrib
      if attributes.get("name") == "always_show_image" or attributes.get("name") == "always-show-image":
        always_show_image = child
      elif attributes.get("name") == "image":
        image = child

  if isbutton and image is not None:
    if always_show_image == None:
      always_show_image = etree.Element("property")
      attributes = always_show_image.attrib
      attributes["name"] = "always-show-image"
      always_show_image.text = "True"
      current.insert(insertpos, always_show_image)
    else:
      always_show_image.text = "True"

def enforce_noshared_adjustments(current, adjustments):
  for child in current:
    enforce_noshared_adjustments(child, adjustments)
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "adjustment":
        if child.text in adjustments:
          raise Exception(sys.argv[1] + ': adjustment used more than once', child.text)
        adjustments.add(child.text)

def enforce_no_productname_in_accessible_description(current, adjustments):
  for child in current:
    enforce_no_productname_in_accessible_description(child, adjustments)
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "AtkObject::accessible-description":
        if "%PRODUCTNAME" in child.text:
          raise Exception(sys.argv[1] + ': %PRODUCTNAME used in accessible-description:' , child.text)

with open(sys.argv[1], encoding="utf-8") as f:
  header = f.readline()
  f.seek(0)
  # remove_blank_text so pretty-printed input doesn't disrupt pretty-printed
  # output if nodes are added or removed
  parser = etree.XMLParser(remove_blank_text=True)
  tree = etree.parse(f, parser)
  # make sure <property name="label" translatable="no"></property> stays like that
  # and doesn't change to <property name="label" translatable="no"/>
  for status_elem in tree.xpath("//property[@name='label' and string() = '']"):
    status_elem.text = ""
  root = tree.getroot()

# do some targeted conversion here
# tdf#138848 Copy-and-Paste in input box should not append an ENTER character
if not sys.argv[1].endswith('/multiline.ui'): # let this one alone not truncate multiline pastes
  add_truncate_multiline(root)
replace_button_use_stock(root)
replace_image_stock(root)
remove_check_button_align(root)
remove_check_button_relief(root)
remove_check_button_image_position(root)
remove_spin_button_input_purpose(root)
remove_caps_lock_warning(root)
remove_spin_button_max_length(root)
remove_track_visited_links(root)
remove_label_pad(root)
remove_label_angle(root)
remove_expander_label_fill(root)
remove_expander_spacing(root)
enforce_menubutton_indicator_consistency(root)
enforce_active_in_group_consistency(root)
enforce_entry_text_column_id_column_for_gtkcombobox(root)
remove_entry_shadow_type(root)
remove_double_buffered(root)
remove_label_yalign(root)
remove_skip_pager_hint(root)
remove_gravity(root)
remove_toolbutton_focus(root)
enforce_toolbar_can_focus(root)
enforce_button_always_show_image(root)
enforce_noshared_adjustments(root, set())
enforce_no_productname_in_accessible_description(root, set())

with open(sys.argv[1], 'wb') as o:
  # without encoding='unicode' (and the matching encode("utf8")) we get &#XXXX replacements for non-ascii characters
  # which we don't want to see changed in the output
  o.write(etree.tostring(tree, pretty_print=True, method='xml', encoding='unicode', doctype=header[0:-1]).encode("utf8"))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
