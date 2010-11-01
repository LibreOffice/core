/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
void test()
{
    portion = appendPortion("text");
    tableContext = new TableContext();
    {
        tableStack.push(tableContext);
        portion = appendPortion("A1");
        tableContext.addPortion(portion);
        tableContext.endCell();
        portion = appendPortion("B1");
        tableContext.addPortion(portion);
        tableContext.endCell();
        portion = appendPortion("C1");
        tableContext.addPortion(portion);
        tableContext.endCell();
        tableContext.endRow(rowProperties);
        portion = appendPortion("A2");
        tableContext.addPortion(portion);
        tableContext.endCell();
        tableContext = new TableContext();
        {
            tableStack.push(tableContext);
            portion = appendPortion("B2A1");
            tableContext.addPortion(portion);
            tableContext.endCell();
            portion = appendPortion("B2B1");
            tableContext.addPortion(portion);
            tableContext.endCell();
            tableContext.endRow(rowProperties);
            portion = appendPortion("B2A2");
            tableContext.addPortion(portion);
            tableContext.endCell();
            portion = appendPortion("B2B2");
            tableContext.addPortion(portion);
            tableContext.endCell();
            tableContext.endRow(rowProperties);
        }
        portion = createTable(tableContext);
        tableContext = tableStack.pop();
        tableContext.addPortion(portion);
        portion = appendPortion("B2");
        tableContext.addPortion(portion);
        tableContext.endCell();
        portion = appendPortion("C2");
        tableContext.addPortion(portion);
        tableContext.endCell();
        tableContext.endRow(rowProperties);
    }
    portion = createTable(tableContext);
    portion = appendPortion("text");
}

/*
  tableContext.endCell:
    merge text ranges of portions to one and add this range to ranges of row.
 */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
