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
    insertpos = insertpos + 1;
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
    insertpos = insertpos + 1;
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

def remove_label_fill(current):
  label_fill = None
  isexpander = current.get('class') == "GtkExpander"
  for child in current:
    remove_label_fill(child)
    if not isexpander:
        continue
    if child.tag == "property":
      attributes = child.attrib
      if attributes.get("name") == "label_fill" or attributes.get("name") == "label-fill":
        label_fill = child

  if label_fill != None:
    current.remove(label_fill)

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
remove_track_visited_links(root)
remove_label_fill(root)

with open(sys.argv[1], 'wb') as o:
  # without encoding='unicode' (and the matching encode("utf8")) we get &#XXXX replacements for non-ascii characters
  # which we don't want to see changed in the output
  o.write(etree.tostring(tree, pretty_print=True, method='xml', encoding='unicode', doctype=header[0:-1]).encode("utf8"))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
