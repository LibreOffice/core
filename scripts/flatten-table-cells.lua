-- Copyright the Collabora Online contributors.
--
-- SPDX-License-Identifier: MPL-2.0
--
-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at http://mozilla.org/MPL/2.0/.

-- Pandoc filter:
-- Flatten every table cell to a single inline line so that pandoc can emit a
-- markdown table instead of falling back to a raw HTML table. Join multiple
-- paragraphs and hard line breaks inside a cell with a separator.

local SEPARATOR = pandoc.Str(" ")

-- A pipe-table cell cannot hold a line break.
local scrub_breaks = {
  LineBreak = function() return SEPARATOR end,
  SoftBreak = function() return SEPARATOR end,
}

local function flatten_cell(cell)
  -- Join blocks of the cell, then scrub any inline line breaks.
  local inlines = pandoc.utils.blocks_to_inlines(cell.contents, { SEPARATOR })
  local scrubbed = pandoc.walk_inline(pandoc.Span(inlines), scrub_breaks).content
  cell.contents = { pandoc.Plain(scrubbed) }
  return cell
end

function Table(table)
  local function walk_rows(rows)
    for _, row in ipairs(rows) do
      for _, cell in ipairs(row.cells) do
        flatten_cell(cell)
      end
    end
  end
  if table.head then walk_rows(table.head.rows) end
  for _, body in ipairs(table.bodies) do
    walk_rows(body.head) -- intermediate header rows
    walk_rows(body.body)
  end
  if table.foot then walk_rows(table.foot.rows) end
  return table
end

