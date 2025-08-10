import click
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from enum import Enum


@dataclass(frozen=True, order=True)
class ThemeColor:
    themeId: int
    tint: float


@dataclass(frozen=True, order=True)
class Font:
    bold: bool
    color: ThemeColor


@dataclass(frozen=True, order=True)
class Fill:
    fgColor: ThemeColor
    bgColor: ThemeColor


class BorderStyle(Enum):
    THIN = "thin"
    MEDIUM = "medium"
    THICK = "thick"
    DOUBLE = "double"

    def __lt__(self, other: "BorderStyle") -> bool:
        if self == other:
            return False

        for elem in BorderStyle:
            if self == elem:
                return True
            elif other == elem:
                return False

        assert False

    def __gt__(self, other):
        if self == other:
            return False

        return not (self < other)

    def __ge__(self, other):
        if self == other:
            return True
        return not (self < other)

    @classmethod
    def lookup(cls, name: str) -> "BorderStyle":
        for e in cls:
            if e.value == name:
                return e

        assert False

    def get_cpp_enum(self) -> str:
        if self == BorderStyle.THIN:
            return "BorderElementStyle::THIN"
        elif self == BorderStyle.MEDIUM:
            return "BorderElementStyle::MEDIUM"
        elif self == BorderStyle.THICK:
            return "BorderElementStyle::THICK"
        elif self == BorderStyle.DOUBLE:
            return "BorderElementStyle::DOUBLE"

@dataclass(frozen=True, order=True)
class BorderElement:
    style: BorderStyle
    color: ThemeColor


@dataclass(frozen=True)
class Border:
    top: BorderElement | None
    bottom: BorderElement | None
    left: BorderElement | None
    right: BorderElement | None
    horizontal: BorderElement | None
    vertical: BorderElement | None

    def get_elements(self):
        return (self.top, self.bottom, self.left, self.right, self.horizontal, self.vertical)

    def get_all_colors(self):
        colors = []
        for field in self.get_elements():
            if field is not None:
                colors.append(field.color)
        return colors

    def __lt__(self, other: "Border") -> bool:
        for self_element, other_element in zip(self.get_elements(), other.get_elements()):
            if self_element == other_element:
                continue

            if self_element is None:
                return True
            elif other_element is None:
                return False
            else:
                return self_element < other_element

        return False

    def __gt__(self, other):
        if self == other:
            return False

        return not (self < other)

    def __ge__(self, other):
        if self == other:
            return True
        return not (self < other)


@dataclass(frozen=True)
class Dxf:
    font: Font
    fill: Fill
    border: Border

    def get_elements(self):
        return self.font, self.fill, self.border

    def __lt__(self, other: "Dxf") -> bool:
        for self_element, other_element in zip(self.get_elements(), other.get_elements()):
            if self_element == other_element:
                continue

            if self_element is None:
                return True
            elif other_element is None:
                return False
            else:
                return self_element < other_element

        return False

    def __gt__(self, other):
        if self == other:
            return False

        return not (self < other)

    def __ge__(self, other):
        if self == other:
            return True
        return not (self < other)


def parse_color(color) -> ThemeColor:
    themeId = int(color.get("theme", "-1"))
    tint = float(color.get("tint", "0.0"))
    assert themeId >= 0
    return ThemeColor(themeId, tint)


def parse_fill(fill) -> Fill:
    patternFill = fill.find("patternFill")
    assert patternFill is not None
    fgColor = patternFill.find("fgColor")
    assert fgColor is not None
    bgColor = patternFill.find("bgColor")
    assert bgColor is not None

    return Fill(parse_color(fgColor), parse_color(bgColor))


def parse_font(font) -> Font:
    is_bold = font.find("b") is not None
    color = font.find("color")
    assert color is not None
    return Font(is_bold, parse_color(color))


def parse_border_element(border_element) -> BorderElement:
    style = border_element.get("style")
    assert style is not None
    color = border_element.find("color")
    assert color is not None
    return BorderElement(BorderStyle.lookup(style), parse_color(color))

def parse_border(border) -> Border:
    elements = []
    for dirs in ["top", "bottom", "left", "right", "horizontal", "vertical"]:
        border_element = border.find(dirs)
        if border_element is None:
            elements.append(None)
        else:
            elements.append(parse_border_element(border_element))

    assert len(elements) == 6
    return Border(*elements)


def parse_dxf(dxf) -> Dxf:
    fill = None
    border = None
    font = None
    for formattingElement in list(dxf):
        if formattingElement.tag == "fill":
            fill = parse_fill(formattingElement)
        elif formattingElement.tag == "border":
            border = parse_border(formattingElement)
        elif formattingElement.tag == "font":
            font = parse_font(formattingElement)
        else:
            assert False

    return Dxf(font, fill, border)


def parse_dxfs(dxfs) -> list[Dxf]:
    return [parse_dxf(dxf) for dxf in list(dxfs)]


class TableStyleType(Enum):
    WHOLE_TABLE = "wholeTable"
    FIRST_COLUMN_STRIPE = "firstColumnStripe"
    SECOND_COLUMN_STRIPE = "secondColumnStripe"
    FIRST_ROW_STRIPE = "firstRowStripe"
    SECOND_ROW_STRIPE = "secondRowStripe"
    LAST_COLUMN = "lastColumn"
    FIRST_COLUMN = "firstColumn"
    HEADER_ROW = "headerRow"
    TOTAL_ROW = "totalRow"
    FIRST_HEADER_CELL = "firstHeaderCell"
    LAST_HEADER_CELL = "lastHeaderCell"
    FIRST_TOTAL_CELL = "firstTotalCell"
    LAST_TOTAL_CELL = "lastTotalCell"

    @classmethod
    def lookup(cls, name: str) -> "TableStyleType":
        for e in cls:
            if e.value == name:
                return e

        assert False

    def get_cpp_enum(self) -> str:
        cpp_map = {TableStyleType.WHOLE_TABLE: "WholeTable", TableStyleType.FIRST_COLUMN_STRIPE: "FirstColumnStripe", TableStyleType.SECOND_COLUMN_STRIPE: "SecondColumnStripe", TableStyleType.FIRST_ROW_STRIPE: "FirstRowStripe", TableStyleType.SECOND_ROW_STRIPE: "SecondRowStripe", TableStyleType.LAST_COLUMN: "LastColumn", TableStyleType.FIRST_COLUMN: "FirstColumn", TableStyleType.HEADER_ROW: "HeaderRow", TableStyleType.TOTAL_ROW: "TotalRow", TableStyleType.FIRST_HEADER_CELL: "FirstHeaderCell", TableStyleType.LAST_HEADER_CELL: "LastHeaderCell", TableStyleType.FIRST_TOTAL_CELL: "FirstTotalCell", TableStyleType.LAST_TOTAL_CELL: "LastTotalCell"}

        return "ScTableStyleElement::" + cpp_map[self]

    def __lt__(self, other: "TableStyleType") -> bool:
        if self == other:
            return False

        for elem in TableStyleType:
            if self == elem:
                return True
            elif other == elem:
                return False

        assert False

    def __gt__(self, other: "TableStyleType"):
        return not (self < other)

    def __ge__(self, other: "TableStyleType"):
        if self == other:
            return True
        return not (self < other)


@dataclass(frozen=True, order=True)
class TableStyleElement:
    dxf: Dxf
    element: TableStyleType


@dataclass(frozen=True, order=True)
class TableStyle:
    name: str
    elements: list[TableStyleElement]


def parse_table_style_element(table_style_element, dxfs: list[Dxf]) -> TableStyleElement:
    table_style_element_type = TableStyleType.lookup(table_style_element.get("type"))
    # translate from 1 based file format to 0 based indexing
    dxfId = int(table_style_element.get("dxfId")) - 1
    assert dxfId >= 0 and dxfId < len(dxfs)

    return TableStyleElement(dxfs[dxfId], table_style_element_type)


def parse_table_style(style) -> TableStyle:
    dxfs = style.find("dxfs")
    style_defs = parse_dxfs(dxfs)
    tableStyles = style.find("tableStyles")
    assert tableStyles is not None
    tableStyle = tableStyles.find("tableStyle")
    assert tableStyle is not None

    table_style_name = tableStyle.get("name")
    if table_style_name == "TableStyleDark9 2":
        # this looks like a bug in the OOXML spec documents
        table_style_name = "TableStyleDark9"

    return TableStyle(table_style_name, [parse_table_style_element(element, style_defs) for element in list(tableStyle)])

def parse_pivot_style(style):
    print("Skipping pivot style definition")


def parse_style(style) -> TableStyle | None:
    print(f"Parsing {style.tag}")

    tableStyles = style.find("tableStyles")
    assert tableStyles is not None
    assert int(tableStyles.get("count", "0")) == 1

    tableStyle = tableStyles.find("tableStyle")
    assert tableStyle is not None

    isTableStyle = int(tableStyle.get("table", "1"))
    isPivotStyle = int(tableStyle.get("pivot", "1"))
    assert isTableStyle or isPivotStyle
    assert not (isTableStyle and isPivotStyle)

    if isTableStyle:
        return parse_table_style(style)

    return None


header = """
/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// WARNING: This file is automatically generated and should not be manually edited!

#include <defaulttablestyles.hxx>

"""

footer = "\n/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */"


def dump_colors(dxfs: set[Dxf]) -> tuple[str, dict[ThemeColor, int]]:
    colors: set[ThemeColor] = set()

    # first collect all ThemeColor objects
    for dxf in dxfs:
        if dxf.fill is not None:
            colors.add(dxf.fill.bgColor)
            colors.add(dxf.fill.fgColor)
        if dxf.font is not None:
            colors.add(dxf.font.color)
        if dxf.border is not None:
            colors.update(dxf.border.get_all_colors())

    currentId = 0
    colorMap: dict[ThemeColor, int] = {}
    themeColors = []
    for color in sorted(colors):
        colorMap[color] = currentId
        themeColors.append(f"{{ {color.themeId}, {color.tint} }}")
        currentId += 1

    themeColorsStr = ", ".join(themeColors)

    colorStr = f"constexpr ThemeColor aThemeColors[] = {{ {themeColorsStr} }};\n"

    return colorStr, colorMap


def dump_fills(dxfs: set[Dxf], color_map: dict[ThemeColor, int]) -> tuple[str, dict[Fill, int]]:
    fills: set[Fill] = set()

    for dxf in dxfs:
        if dxf.fill is not None:
            fills.add(dxf.fill)

    currentId = 0
    fill_map: dict[Fill, int] = {}
    fillOutputs = []
    for fill in sorted(fills):
        fill_map[fill] = currentId
        fg_color_id = color_map[fill.fgColor]
        bg_color_id = color_map[fill.bgColor]
        fillOutputs.append(f"{{ {fg_color_id}, {bg_color_id} }}")
        currentId += 1

    fillOutputStr = ", ".join(fillOutputs)

    fillStr = f"constexpr Fill aFills[] = {{ {fillOutputStr} }};\n"

    return fillStr, fill_map

def dump_fonts(dxfs: set[Dxf], color_map: dict[ThemeColor, int]) -> tuple[str, dict[Font, int]]:
    fonts: set[Font] = set()

    for dxf in dxfs:
        if dxf.font is not None:
            fonts.add(dxf.font)

    currentId = 0
    font_map: dict[Font, int] = {}
    fontOutputs = []
    for font in sorted(fonts):
        font_map[font] = currentId
        bold = "true" if font.bold else "false"
        color_id = color_map[font.color]
        fontOutputs.append(f"{{ {bold}, {color_id} }}")
        currentId += 1

    fontOutputStr = ", ".join(fontOutputs)

    fontStr = f"constexpr Font aFonts[] = {{ {fontOutputStr} }};\n"

    return fontStr, font_map


def dump_border_elements(borders: set[Border], color_map: dict[ThemeColor, int]) -> tuple[str, dict[BorderElement, int]]:
    border_elements: set[BorderElement] = set()

    for border in borders:
        for border_element in (border.top, border.bottom, border.left, border.right, border.horizontal, border.vertical):
            if border_element is None:
                continue

            border_elements.add(border_element)

    currentId = 0
    border_element_map: dict[Font, int] = {}
    border_element_outputs = []
    for border_element in sorted(border_elements):
        border_element_map[border_element] = currentId
        color_id = color_map[border_element.color]
        border_style = border_element.style.get_cpp_enum()
        border_element_outputs.append(f"{{ {border_style}, {color_id} }}")
        currentId += 1

    border_element_output_str = ", ".join(border_element_outputs)

    border_element_str = f"constexpr BorderElement aBorderElements[] = {{ {border_element_output_str} }};\n"

    return border_element_str, border_element_map


def dump_borders(dxfs: set[Dxf], color_map: dict[ThemeColor, int]) -> tuple[str, dict[Border, int]]:
    borders: set[Border] = set()

    for dxf in dxfs:
        if dxf.border is not None:
            borders.add(dxf.border)

    border_element_str, border_element_map = dump_border_elements(borders, color_map)

    currentId = 0
    border_map: dict[Border, int] = {}
    border_output = []
    for border in sorted(borders):
        b_str = []
        for element in (border.top, border.bottom, border.left, border.right, border.horizontal, border.vertical):
            if element is None:
                b_str.append("-1")
            else:
                border_element_id = border_element_map[element]
                b_str.append(f"{border_element_id}")

        border_map[border] = currentId
        border_str = ", ".join(b_str)
        border_output.append(f"{{ {border_str} }}")
        currentId += 1

    border_output_str = ", ".join(border_output)

    b_output = f"{border_element_str}\nconstexpr Border aBorders[] = {{ {border_output_str} }};\n"

    return b_output, border_map


def dump_dxfs(dxfs: set[Dxf], fills_map: dict[Fill, int], font_map: dict[Font, int], border_map: dict[Border, int]) -> tuple[str, dict[Dxf, int]]:
    dxf_output = []
    dxf_map: dict[Dxf, int] = {}

    currentId = 0
    for dxf in sorted(dxfs):
        fill_id = fills_map[dxf.fill] if dxf.fill is not None else -1
        border_id = border_map[dxf.border] if dxf.border is not None else -1
        font_id = font_map[dxf.font] if dxf.font is not None else -1
        dxf_output.append(f"{{ {fill_id}, {border_id}, {font_id} }}")
        dxf_map[dxf] = currentId

        currentId += 1

    dxf_output_str = ", ".join(dxf_output)
    dxf_str = f"constexpr Dxf aDxfs[] = {{ {dxf_output_str} }};\n"

    return dxf_str, dxf_map


def get_all_dxfs(table_styles: list[TableStyle]) -> set[Dxf]:
    dxfs: set[Dxf] = set()
    for table_style in table_styles:
        dxfs.update([element.dxf for element in table_style.elements])

    return dxfs


def dump_table_style_elements(table_styles: list[TableStyle], dxf_map: dict[Dxf, int]) -> tuple[str, dict[TableStyleElement, int]]:
    table_style_elements: set[TableStyleElement] = set()

    for table_style in table_styles:
        table_style_elements.update(table_style.elements)

    table_style_element_map: dict[TableStyleElement, int] = {}
    output = []
    currentId = 0
    for table_style_element in sorted(table_style_elements):
        dxf_id = dxf_map[table_style_element.dxf]
        element_id = table_style_element.element.get_cpp_enum()
        output.append(f"{{ {element_id}, {dxf_id} }}")
        table_style_element_map[table_style_element] = currentId
        currentId += 1

    output_str = ", ".join(output)
    table_style_element_str = f"constexpr TableStyleElement aTableStyleElements[] = {{ {output_str} }};\n"

    return table_style_element_str, table_style_element_map


def dump_table_styles(table_styles: list[TableStyle], output_path: str):
    content = [header]
    dxf_set = get_all_dxfs(table_styles)

    color_str, color_map = dump_colors(dxf_set)
    content.append(color_str)

    fills_str, fills_map = dump_fills(dxf_set, color_map)
    content.append(fills_str)

    font_str, font_map = dump_fonts(dxf_set, color_map)
    content.append(font_str)

    border_str, border_map = dump_borders(dxf_set, color_map)
    content.append(border_str)

    dxf_str, dxf_map = dump_dxfs(dxf_set, fills_map, font_map, border_map)
    content.append(dxf_str)

    table_style_element_str, table_style_element_map = dump_table_style_elements(table_styles, dxf_map)
    content.append(table_style_element_str)

    output = []
    for table_style in sorted(table_styles):
        table_style_element_ids = ", ".join([f"{table_style_element_map[element]}" for element in table_style.elements])
        table_style_element_str = f"{{ {table_style_element_ids} }}"
        output.append(f"{{ \"{table_style.name}\", {len(table_style.elements)}, {table_style_element_str} }}")

    output_str = ", ".join(output)
    table_style_str = f"constexpr TableStyle aTableStyles[] = {{ {output_str} }};\n"
    content.append(table_style_str)

    content.append(footer)
    with open(output_path, "w") as f:
        f.write("\n".join(content))


@click.command()
@click.argument("file_path", type=str, required=True)
@click.argument("output_path", type=str, required=True)
def parse_table_style_document(file_path, output_path):
    tree = ET.parse(file_path)
    root = tree.getroot()
    table_styles = [parse_style(style) for style in list(root)]
    table_styles = list(filter(lambda x: x is not None, table_styles))

    dump_table_styles(table_styles, output_path)

if __name__ == "__main__":
    parse_table_style_document()
