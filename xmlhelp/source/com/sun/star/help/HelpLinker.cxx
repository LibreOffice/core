/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HelpLinker.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 13:55:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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

#include "HelpCompiler.hxx"

#include <map>

#include <string.h>
#include <limits.h>

#include <boost/shared_ptr.hpp>
#include <boost/tokenizer.hpp>

#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/functions.h>
#include <libxslt/extensions.h>

#include <unicode/brkiter.h>
#include <unicode/ustring.h>
#include <unicode/ucnv.h>

#include <sal/types.h>
#include <osl/time.h>

class JarOutputStream
{
private:
    fs::path filename;
    std::ostringstream perlline;
public:
    JarOutputStream();
    void setname(const fs::path &name) { filename = name; }
    const fs::path& getname() const { return filename; }
    void addFile(const std::string &name, const std::string &key);
    void addTree(const std::string &dir, const std::string &key);
    void dontCompress(const std::string &key);
    void commit();
};

struct Data
{
    std::vector<std::string> _idList;
    typedef std::vector<std::string>::const_iterator cIter;

    void append(const std::string &id)
    {
        _idList.push_back(id);
    }

    std::string getString() const
    {
        std::string ret;
        cIter aEnd = _idList.end();
        for (cIter aIter = _idList.begin(); aIter != aEnd; ++aIter)
            ret += *aIter + ";";
        return ret;
    }
};

class HelpKeyword
{
private:
    typedef std::hash_map<std::string, Data, pref_hash> DataHashtable;
    DataHashtable _hash;

public:
    void insert(const std::string &key, const std::string &id)
    {
        Data &data = _hash[key];
        data.append(id);
    }

    void dump(DB* table)
    {
        DataHashtable::const_iterator aEnd = _hash.end();
        for (DataHashtable::const_iterator aIter = _hash.begin(); aIter != aEnd; ++aIter)
        {
            const std::string &keystr = aIter->first;
            DBT key;
            memset(&key, 0, sizeof(key));
            key.data = const_cast<char*>(keystr.c_str());
            key.size = keystr.length();

            const Data &data = aIter->second;
            std::string str = data.getString();
            DBT value;
            memset(&value, 0, sizeof(value));
            value.data = const_cast<char*>(str.c_str());
            value.size = str.length();

            table->put(table, NULL, &key, &value, 0);
        }
    }
};

namespace PrefixTranslator
{
    std::string translatePrefix(const std::string &input)
    {
        if (input.find("vnd.sun.star.help://") == 0)
            return std::string("#HLP#") + input.substr(strlen("vnd.sun.star.help://"));
        else
            return input;
    }
}

class IndexAccessor
{
    fs::path _dirName;
public:
    IndexAccessor(const fs::path &dirName) : _dirName(dirName) {}
    IndexAccessor(const IndexAccessor &another) { _dirName = another._dirName; }
    fs::path indexFile(const std::string &name) const { return _dirName / name; }
    std::ifstream* getLineInput(const std::string &name);
    std::fstream* getOutputStream(const std::string &name);
    std::vector<unsigned char> readByteArray(const std::string &fileName);
    void clear();
    std::fstream *getRAF(const std::string &name, bool update) throw( HelpProcessingException );
    void createIfNeeded() {}
};

std::ifstream* IndexAccessor::getLineInput(const std::string &name)
{
    return new std::ifstream(indexFile(name).native_file_string().c_str());
}

std::fstream* IndexAccessor::getOutputStream(const std::string &name)
{
    return new std::fstream(indexFile(name).native_file_string().c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
}

std::vector<unsigned char> IndexAccessor::readByteArray(const std::string &fileName)
{
    std::ifstream in(indexFile(fileName).native_file_string().c_str(), std::ios::binary);
    std::vector<unsigned char> ret(1024*16);
    int i=0;
    while (in.good())
    {
        int len = in.readsome((char *)&ret[i], 1024*16);
        if (!len)
            break;
        i += len;
        ret.resize(i+1024*16);
    }
    ret.resize(i);
    return ret;
}

std::fstream* IndexAccessor::getRAF(const std::string &name, bool update)
    throw( HelpProcessingException )
{
    std::fstream *_file = new std::fstream;
    fs::path fullname = indexFile(name);
    if (!update)
    {
        _file->open(fullname.native_file_string().c_str(), std::ios::in | std::ios::binary);
    }
    else
    {
        _file->open(fullname.native_file_string().c_str(), std::ios::in | std::ios::out | std::ios::binary);
        if (!_file->is_open())
        {
            HCDBG(std::cerr << "didn't exist" << std::endl);
            _file->open(fullname.native_file_string().c_str(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
        }
        if (!_file->is_open())
        {
            std::stringstream aStrStream;
            aStrStream << "Cannot open " << name << std::endl;
            throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
        }
    }
    return _file;
}

void IndexAccessor::clear()
{
#if 0
    File thisDir = indexFile(".");
    File[] components = thisDir.listFiles();
    if (components != null)
        for (int i = 0; i < components.length; i++)
            components[i].delete();
#endif
}

typedef std::vector< std::string > VectorLines;

class Schema : public IndexAccessor
{
private:
    static std::string PartName;
    bool _update;
    VectorLines _lines;
public:
    Schema(const IndexAccessor &index, bool update);
    std::ifstream* getSchemaLineInput() { return getLineInput(PartName); }
    void read();
    Stringtable parameters(const std::string &name) const;
    void update(const std::string &partName, const std::string &parameters);
    void save();
};

std::string Schema::PartName = "SCHEMA";


class startsWith
{
public:
    startsWith(const std::string &in) : str(in) {}
    bool operator() ( const std::string &in ) const { return (in.find(str) == 0); }
private:
    const std::string &str;
};

void Schema::update(const std::string &partName, const std::string &inparameters)
{
    VectorLines::iterator aEnd = std::remove_if(_lines.begin(), _lines.end(), startsWith(partName));
    if (aEnd != _lines.end()) _lines.erase(aEnd, _lines.end());
    _lines.push_back(partName + " " + inparameters);
}

Stringtable Schema::parameters(const std::string &name) const
{
    Stringtable result;
    VectorLines::const_iterator aEnd = _lines.end();
    for (VectorLines::const_iterator aIter = _lines.begin(); aIter != aEnd; ++aIter)
    {
        if (aIter->find(name) == 0)
        {
            boost::char_separator<char> sep(" =");
            boost::tokenizer< boost::char_separator<char> > tokens(name, sep);
            boost::tokenizer< boost::char_separator<char> >::const_iterator it = tokens.begin();
            ++it;             // skip name
            while(it != tokens.end())
            {
                const std::string &part1 = *it;
                ++it;
                if (it == tokens.end())
                    break;
                const std::string &part2 = *it;
                result[part1] = part2;
                ++it;
            }
            break;
        }
    }
    return result;
}

Schema::Schema(const IndexAccessor &index, bool inupdate) : IndexAccessor(index),
    _update(inupdate)
{
    read();
}

#ifdef UNX
#define MAX_LINE PATH_MAX
#else
#define MAX_LINE _MAX_PATH
#endif

void Schema::read()
{
    std::ifstream* in = getSchemaLineInput();
    char line[MAX_LINE];
    // This needs to be replaced with our XML Parser
    while (in->getline(line, MAX_LINE))
        _lines.push_back(line);
    delete in;
}

void Schema::save()
{
    if (_update)
    {
        std::fstream* out = getOutputStream(PartName);
        *out << "JavaSearch 1.0\n";
        VectorLines::const_iterator aEnd = _lines.end();
        for (VectorLines::const_iterator aIter = _lines.begin(); aIter != aEnd; ++aIter)
            *out << *aIter << '\n';
        delete out;
    }
}

class DBPartParameters
{
    Schema &_schema;
    std::string _partName;
    Stringtable _parameters;
protected:
    bool parametersKnown() const;
    void updateSchema(const std::string &parameters) { _schema.update(_partName, parameters); }
public:
    DBPartParameters(Schema &schema, const std::string &partName);
    int integerParameter(const std::string &name);
};

DBPartParameters::DBPartParameters(Schema &schema, const std::string &partName)
    : _schema(schema),  _partName(partName)
{
    _parameters = schema.parameters(partName);
}

bool DBPartParameters::parametersKnown() const
{
    return !_parameters.empty();
}

int DBPartParameters::integerParameter(const std::string &name)
{
    std::istringstream converter(_parameters[name]);
    int ret;
    converter >> ret;
    return ret;
}

class BlockManagerParameters : public DBPartParameters
{
private:
    fs::path _file;
    int _blockSize;
protected:
    int _root;
public:
    BlockManagerParameters(Schema &schema, const std::string &partName);
    bool readState();
    const fs::path& getFile() const { return _file; }
    int getBlockSize() const { return _blockSize; }
    void setBlockSize(int size) { _blockSize = size; }
    int getRootPosition() const { return _root; }
    void setRoot(int root) { _root = root; }
    void updateSchema(const std::string &params);
};

void BlockManagerParameters::updateSchema(const std::string &params)
{
    std::ostringstream tmp;
    tmp << "bs=" << _blockSize << " rt=" << _root << " fl=-1 " << params;
    DBPartParameters::updateSchema(tmp.str());
}

BlockManagerParameters::BlockManagerParameters(Schema &schema, const std::string &partName)
    : DBPartParameters(schema, partName), _root(0)
{
      _file = schema.indexFile(partName);
      HCDBG(std::cerr << "file name set to " << _file.native_file_string());
      readState();
}

bool BlockManagerParameters::readState()
{
    if (parametersKnown())
    {
        _blockSize = integerParameter("bs");
        _root = integerParameter("rt");
        return true;
    }
    else
        return false;
}

class BtreeDictParameters : public BlockManagerParameters
{
private:
    int _id1;
public:
    BtreeDictParameters(Schema &schema, const std::string &partName);
    int getFreeID() const { return _id1; }
    void setFreeID(int id) { _id1 = id; }
    void updateSchema();
};

void BtreeDictParameters::updateSchema()
{
    std::ostringstream tmp;
    tmp << "id1=" << _id1 << " id2=1";
    BlockManagerParameters::updateSchema(tmp.str());
}

BtreeDictParameters::BtreeDictParameters(Schema &schema, const std::string &partName)
    : BlockManagerParameters(schema, partName)
{
}

int readInt(std::fstream &in)
{
    HCDBG(std::cerr << "want to read at " << in.tellg() << std::endl);
    int ret = 0;
    for (int i = 3; i >= 0; --i)
    {
        unsigned char byte;
        in.read( (char*)&byte, 1 );
        ret |= (static_cast<unsigned int>(byte) << (i*8));
        HCDBG(fprintf(stderr, "inputting %x ret is now %x\n", byte, ret));
    }
    return ret;
}

void writeByte(std::fstream &out, unsigned char byte)
{
    out.write( (const char *)&byte, 1 );
}

void writeShort(std::fstream &out, int item)
{
    for (int i = 1; i >= 0; --i)
    {
        unsigned char byte = static_cast<unsigned char>((item >> (i*8)));
        out.write( (const char*)&byte, 1 );
    }
}

void writeInt(std::fstream &out, int item)
{
    HCDBG(std::cerr << "want to write at " << out.tellp() << std::endl);
    for (int i = 3; i >= 0; --i)
    {
        unsigned char byte = static_cast<unsigned char>((item >> (i*8)));
        HCDBG(fprintf(stderr, "outputting %x in is %x\n", byte, item));
        out.write( (const char*)&byte, 1 );
    }
}

void readFully(std::fstream &in, std::vector<unsigned char> &_data)
{
    in.read((char*)(&_data[0]), _data.size());
}

/**

   Base class for (typically btree) blocks to hold either
   byte vectors representing graph/tree edges,
   or pairs (key, id) for dictionaries

   Each block has a header and a data section

 */

class Block
{
public:
    static int HEADERLEN;
    // length of Block ID in bytes
    static int IDLEN;

    // number of the block
    // used for both referring to the block
    // and addresssing the block in file
    unsigned int _number;
    bool _isLeaf;
    // first available byte in data section
    int  _free;
    std::vector<unsigned char> _data;

    Block(int blocksize) : _number(0), _isLeaf(true), _free(0)
    {
        _data.resize(blocksize - HEADERLEN);
    }

    virtual ~Block() {}

    void setBlockNumber(int n) { _number = n; }
    virtual void setFree(int free) { _free = free; }
    // interpret 4 bytes at 'i' as an integer
    int integerAt(int i) const
    {
        int result = ((((((_data[i]&0xFF)<<8)
                   |_data[i+1]&0xFF)<<8)
                 |_data[i+2]&0xFF)<<8)
            |_data[i+3]&0xFF;
        return result;
    }
    void setIntegerAt(int i, int value)
    {
        /*
        for (int j = i + 3; j >= i; j--, value >>= 8)
            _data[j] = (unsigned char)(value & 0xFF);
        */
        _data[i++] = (unsigned char)((value >> 24) & 0xFF);
        _data[i++] = (unsigned char)((value >> 16) & 0xFF);
        _data[i++] = (unsigned char)((value >>  8) & 0xFF);
        _data[i]   = (unsigned char)(value & 0xFF);
    }
    void readIn(std::fstream &in)
    {
        _number = readInt(in);
        int twoFields = readInt(in);
        _isLeaf = (twoFields & 0x80000000) != 0;
        HCDBG(std::cerr << "read leaf as " << _isLeaf << std::endl);
        _free = twoFields & 0x7FFFFFFF;
        readFully(in, _data);
    }
    void writeOut(std::fstream &out) const
    {
        writeInt(out, _number);
        writeInt(out, _free | (_isLeaf ? 0x80000000 : 0));
        out.write((const char*)(&_data[0]), _data.size());
    }
};

int Block::HEADERLEN = 8;
// length of Block ID in bytes
int Block::IDLEN = 4;

class BtreeDict;
class EntryProcessor;
typedef std::vector<int> IntegerArray;

class DictBlock : public Block
{
public:
    DictBlock();
    int free() const { return _free + firstEntry(); }
    int numberOfEntries() const { return integerAt(0); }
    int nthPointer(int n) const { return integerAt(4*(n + 1)); }
    int getChildIdx(int index) const;
    int entryKeyLength(int i) const { return _data[i] & 0xFF; }
    int entryCompression(int i) const { return _data[i + 1] & 0xFF; }
    int entryID(int i) const { return integerAt(i + 2); }
    int entryLength(int entry) const;
    int entryKey(int entry) const;
    int firstEntry() const { return 4; }
    int nextEntry(int entry) const { return entry + entryLength(entry); }
    void restoreKeyInBuffer(int entry, std::vector<unsigned char> &buffer);
    std::string restoreKey(int entry, std::vector<unsigned char> &buffer);
    std::string findID(int id) throw( HelpProcessingException );
    void setBlockNumbers(std::vector<int> &blocks) const;
    void listBlock();
    void doMap(BtreeDict &owner, const EntryProcessor &processor);
    void withPrefix(BtreeDict &owner, const std::string &prefix,
        size_t prefLen, IntegerArray &result);
};

class BlockFactory;

class BlockProcessor;

class BlockDescriptor
{
public:
    Block *_block;
    bool _modf;
    BlockDescriptor(Block *block) : _block(block), _modf(false) {}
}; // end of BlockDescriptor

class BlockManager
{
private:
    static int INCR;
    std::fstream _file;
    long _blockSize;
    bool _update;
    BlockFactory *_blockFactory;
    std::vector<BlockDescriptor> _blockTab;
public:
    BlockManager(const BlockManagerParameters *params,
        bool update, BlockFactory *bfactory) throw( HelpProcessingException );
    ~BlockManager();
    Block& accessBlock(int blockNumber);
    void setModified(int blNum);
    void close();
    Block& getNewBlock();
    void processBlocks(BlockProcessor &processor);
    void mapBlock(Block* block);
    void addDescriptor(Block* block) throw( HelpProcessingException );
private:
    void writeBlock(const Block &bl);
};

int BlockManager::INCR = 64; // size increment

class EntryProcessor
{
public:
    virtual void processEntry(const std::string &string, int id) const = 0;
    virtual ~EntryProcessor() {};
};

class BtreeDict
{
public:
    static int ENTHEADERLEN;
    static int BLOCKSIZE;
    static int DATALEN;
    static int MaxKeyLength;
    static int lastPtrIndex;
protected:
    BlockManager *blockManager;
    int          root;
    std::vector<int> blocks;

    BtreeDict() {/*empty*/}
    ~BtreeDict() { delete blockManager; }
    BtreeDict(const BtreeDictParameters *params);
    void init(const BtreeDictParameters *params, bool update,
        BlockFactory *bfactory);
public:
    int fetch(const std::string &key);
    void close();
private:
    std::string fetch(int conceptID);
    IntegerArray withPrefix(const std::string &prefix);
public:
    DictBlock& accessBlock(int index);
    DictBlock& child(const DictBlock &bl, int index) throw( HelpProcessingException );
private:
    std::string findID(int blNum, int id);
    int find(const DictBlock &bl, std::vector<unsigned char> &key, int index);
    int find(const DictBlock &bl, std::vector<unsigned char> &key);
    void setBlocks(std::vector<int> &blocks);
    void map(const EntryProcessor &processor);
public:
    void dumpnode(DictBlock &bl, int level);
};

class BlockFactory
{
public:
    virtual Block* makeBlock() const = 0;
    virtual ~BlockFactory() {}
};

static int dictcount;

class DictBlockFactory : public BlockFactory
{
public:
    Block* makeBlock() const
    {
        dictcount++;
        return new DictBlock;
    }
};

BtreeDict::BtreeDict(const BtreeDictParameters *params)
{
    init(params, false, new DictBlockFactory());
    blocks.resize(params->getFreeID());
    setBlocks(blocks);
}

void BtreeDict::dumpnode(DictBlock &bl, int level)
{
    if (!bl._isLeaf)
    {
        fprintf(stderr, "\n");
        for (int i = 0; i < level; ++i)
            fprintf(stderr, "\t");
        fprintf(stderr, "there are %d entries\n", bl.numberOfEntries());
        for (int i = 0; i < level; ++i)
            fprintf(stderr, "\t");
        for (int i = 0; i < bl.numberOfEntries(); ++i)
        {
            int index = bl.getChildIdx(i);
            fprintf(stderr, " %d ", index);
            DictBlock &thischild = accessBlock(index);
            dumpnode(thischild, level + 1);
        }
        fprintf(stderr, "\n");
    }
}

int BtreeDict::fetch(const std::string &key)
{
    HCDBG(std::cerr << "fetching " << key << " from root " << root << std::endl);
    DictBlock &bl = accessBlock(root);

    int length = key.size();
    std::vector<unsigned char> Key(length + 1);
    memcpy(&(Key[0]), key.c_str(), length);
    Key[length] = 0;            // sentinel

    return find(bl, Key);
}

std::string BtreeDict::fetch(int conceptID)
{
    return findID(blocks[conceptID], conceptID);
}

IntegerArray BtreeDict::withPrefix(const std::string &prefix)
{
    IntegerArray result;
    accessBlock(root).withPrefix(*this, prefix, prefix.size(), result);
    return result;
}

void BtreeDict::close()
{
    blockManager->close();
}

void BtreeDict::init(const BtreeDictParameters *params, bool update,
    BlockFactory *bfactory)
{
      blockManager = new BlockManager(params, update, bfactory);
      root = params->getRootPosition();
}

DictBlock& BtreeDict::accessBlock(int index)
{
    return (DictBlock&)blockManager->accessBlock(index);
}

DictBlock& BtreeDict::child(const DictBlock &bl, int index) throw( HelpProcessingException )
{
    if (bl._isLeaf)
    {
        std::stringstream aStrStream;
        aStrStream << "leaf's can't have children, screwed!" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_INTERNAL_ERROR, aStrStream.str() );
    }
    return accessBlock(bl.getChildIdx(index));
}

std::string BtreeDict::findID(int blNum, int id)
{
    return accessBlock(blNum).findID(id);
}

int BtreeDict::find(const DictBlock &bl, std::vector<unsigned char> &key, int index)
{
    HCDBG(std::cerr << "find2: " << bl._isLeaf << " : " << index << " : " << std::endl);

    return bl._isLeaf ? 0 : find(child(bl, index), key);
}

int BtreeDict::find(const DictBlock &bl, std::vector<unsigned char> &key)
{
    int inputKeyLen = key.size() - 1;
    int entryPtr    = bl.firstEntry();
    int freeSpace   = bl.free();
    int nCharsEqual = 0;
    int compression = 0;

    HCDBG(std::cerr << "find1: " << inputKeyLen << " : "
        << entryPtr << " : " << freeSpace << " : " << nCharsEqual << " "
        << compression << std::endl);

    for (int entryIdx = 0;;)
    {
        if (entryPtr == freeSpace)
            return find(bl, key, bl.numberOfEntries());
        else if (compression == nCharsEqual)
        {
            int keyLen = bl.entryKeyLength(entryPtr);
            int keyPtr = bl.entryKey(entryPtr), i;
            for (i = 0; i < keyLen && key[nCharsEqual] == bl._data[keyPtr + i]; i++)
                ++nCharsEqual;
            if (i == keyLen)
            {
                if (nCharsEqual == inputKeyLen)
                    return bl.entryID(entryPtr);
            }
            else if ((key[nCharsEqual]&0xFF) < (bl._data[keyPtr + i]&0xFF))
                return find(bl, key, entryIdx);
        }
        else if (compression < nCharsEqual) // compression dropped
            return find(bl, key, entryPtr == freeSpace
                    ? bl.numberOfEntries() : entryIdx);
        do
        {
            entryPtr = bl.nextEntry(entryPtr);
            ++entryIdx;
        }
        while (bl.entryCompression(entryPtr) > nCharsEqual);
        compression = bl.entryCompression(entryPtr);
    }
}

class BlockProcessor
{
protected:
    std::vector<int> &blocks;
public:
    BlockProcessor(std::vector<int> &_blocks) : blocks(_blocks) {}
    virtual void process(const Block &block) = 0;
    virtual ~BlockProcessor() {}
};


class DictBlockProcessor : public BlockProcessor
{
public:
    DictBlockProcessor(std::vector<int> &_blocks) : BlockProcessor(_blocks) {}
    void process(const Block &block)
    {
        ((const DictBlock&)block).setBlockNumbers(blocks);
    }
};

BlockManager::BlockManager(const BlockManagerParameters *params,
    bool update, BlockFactory *bfactory) throw( HelpProcessingException )
    : _blockFactory(bfactory)
{
    _update = update;
    //    params.readState();
    _blockSize = params->getBlockSize();
    HCDBG(std::cerr << "opening " << params->getFile().native_file_string() << std::endl);
    if (!update)
    {
        _file.open(params->getFile().native_file_string().c_str(), std::ios::in | std::ios::binary);
    }
    else
    {
        _file.open(params->getFile().native_file_string().c_str(), std::ios::in | std::ios::out | std::ios::binary);
        if (!_file.is_open())
        {
            HCDBG(std::cerr << "didn't exist" << std::endl);
            _file.open(params->getFile().native_file_string().c_str(),
                std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
        }
        if (!_file.is_open())
        {
            std::stringstream aStrStream;
            aStrStream << "Cannot open " << params->getFile().native_file_string() << std::endl;
            throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
        }
    }

    _file.seekg(0, std::ios::end);
    long length = _file.tellg();
    if (length < 0) length = 0;
    _file.seekg(0, std::ios::beg);
    _file.clear();

    HCDBG(std::cerr << "len is " << length << std::endl);

    if (length <= 0 && update)
    {
        Block* _dummy = bfactory->makeBlock();
        _dummy->setBlockNumber(0);
        writeBlock(*_dummy);
        delete _dummy;
        length = _blockSize;
    }

    _file.seekg(0, std::ios::beg);

    int _blockTableSize = (length/_blockSize);
    HCDBG(std::cerr << "len is now " << _blockTableSize << std::endl);
    for (int i = 0; i < _blockTableSize; ++i)
        mapBlock(bfactory->makeBlock());
}

Block& BlockManager::getNewBlock()
{
    unsigned int number = _blockTab.size();

    Block *bl = _blockFactory->makeBlock();
    bl->setBlockNumber(number);
    writeBlock(*bl);
    addDescriptor(bl);

    return *(_blockTab[number]._block);
}

void BlockManager::setModified(int blNum)
{
    _blockTab[blNum]._modf = true;
}

void BlockManager::close()
{
    if (_update)
    {
        std::vector<BlockDescriptor>::const_iterator aEnd = _blockTab.end();
        for (std::vector<BlockDescriptor>::const_iterator aIter = _blockTab.begin();
            aIter != aEnd; ++aIter)
        {
            if (aIter->_modf)
                writeBlock(*(aIter->_block));
        }
    }
    _file.close();
}

void BlockManager::processBlocks(BlockProcessor &processor)
{
    std::vector<BlockDescriptor>::const_iterator aEnd = _blockTab.end();
    for (std::vector<BlockDescriptor>::const_iterator aIter = _blockTab.begin();
        aIter != aEnd; ++aIter)
    {
        processor.process(*(aIter->_block));
    }
}

void BlockManager::mapBlock(Block* block)
{
    block->readIn(_file);
    addDescriptor(block);
}

void BlockManager::addDescriptor(Block *block) throw( HelpProcessingException )
{
    BlockDescriptor desc(block);
    _blockTab.push_back(desc);
    HCDBG(std::cerr << "numbers are " << block->_number << " " << (_blockTab.size()-1) << std::endl);
    if (block->_number != _blockTab.size() - 1)
    {
        std::stringstream aStrStream;
        aStrStream << "totally screwed" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_INTERNAL_ERROR, aStrStream.str() );
    }
    HCDBG(std::cerr << "addDescriptor blocks are now " << _blockTab.size() << std::endl);
}

void BlockManager::writeBlock(const Block &bl)
{
    _file.seekp(_blockSize * bl._number);
    bl.writeOut(_file);
}

Block& BlockManager::accessBlock(int blockNumber)
{
    return *(_blockTab[blockNumber]._block);
}

BlockManager::~BlockManager()
{
    std::vector<BlockDescriptor>::iterator aEnd = _blockTab.end();
    for (std::vector<BlockDescriptor>::iterator aIter = _blockTab.begin();
        aIter != aEnd; ++aIter)
    {
        delete aIter->_block;
    }
    delete _blockFactory;
}

void BtreeDict::setBlocks(std::vector<int> &inblocks)
{
    DictBlockProcessor foo(inblocks);
    blockManager->processBlocks(foo);
}

//    can go to Full
void BtreeDict::map(const EntryProcessor &processor)
{
    accessBlock(root).doMap(*this, processor);
}

void DictBlock::restoreKeyInBuffer(int entry, std::vector<unsigned char> &buffer)
{
    int howMany = entryKeyLength(entry);
    int where = entryCompression(entry);
    int from = entryKey(entry);
    while (howMany-- > 0)
        buffer[where++] = _data[from++];
}

std::string DictBlock::restoreKey(int entry, std::vector<unsigned char> &buffer)
{
    int howMany = entryKeyLength(entry);
    int where = entryCompression(entry);
    int from = entryKey(entry);
    while (howMany-- > 0)
        buffer[where++] = _data[from++];
    return std::string((const char*)(&buffer[0]), 0, where);
}

std::string DictBlock::findID(int id) throw( HelpProcessingException )
{
    std::vector<unsigned char> buffer(BtreeDict::MaxKeyLength);
    int freeSpace = free();
    for (int ent = firstEntry(); ent < freeSpace; ent = nextEntry(ent))
    {
        if (entryID(ent) == id) // found
            return restoreKey(ent, buffer);
        else
            restoreKeyInBuffer(ent, buffer);
    }
    std::stringstream aStrStream;
    aStrStream << "ID not found in block" << std::endl;
    throw HelpProcessingException( HELPPROCESSING_INTERNAL_ERROR, aStrStream.str() );
}

void DictBlock::setBlockNumbers(std::vector<int> &blocks) const
{
    for (int e = firstEntry(); e < _free; e = nextEntry(e))
        blocks[entryID(e)] = _number;
}

void DictBlock::listBlock()
{
    std::vector<unsigned char> buffer(BtreeDict::MaxKeyLength);
    int freeSpace = free();
    int entryPtr = firstEntry();
    if (_isLeaf)
    {
        while (entryPtr < freeSpace)
        {
            std::cout << restoreKey(entryPtr, buffer) << " " <<
                entryID(entryPtr);
            entryPtr = nextEntry(entryPtr);
        }
    }
    else
        std::cout << "not leaf" << std::endl;
}

void DictBlock::doMap(BtreeDict &owner, const EntryProcessor &processor)
{
    std::vector<unsigned char> buffer(BtreeDict::MaxKeyLength);
    int freeSpace = free();
    int entryPtr = firstEntry();
    if (_isLeaf)
    {
        while (entryPtr < freeSpace)
        {
            processor.processEntry(restoreKey(entryPtr, buffer),
                entryID(entryPtr));
            entryPtr = nextEntry(entryPtr);
        }
    }
    else
    {
        int entryIdx  = 0;
        while (entryPtr < freeSpace)
        {
              owner.accessBlock(getChildIdx(entryIdx)).doMap(owner,processor);
              processor.processEntry(restoreKey(entryPtr, buffer),
                    entryID(entryPtr));
              entryPtr = nextEntry(entryPtr);
              ++entryIdx;
        }
        owner.accessBlock(getChildIdx(entryIdx)).doMap(owner, processor);
    }
}

void DictBlock::withPrefix(BtreeDict &owner, const std::string &prefix,
    size_t prefLen, IntegerArray &result)
{
    std::vector<unsigned char> buffer(BtreeDict::MaxKeyLength);
    int freeSpace = free();
    int entryPtr = firstEntry();
    if (_isLeaf)
    {
        while (entryPtr < freeSpace)
        {
            if (restoreKey(entryPtr, buffer).find(prefix) == 0)
                result.push_back(entryID(entryPtr));
            entryPtr = nextEntry(entryPtr);
        }
    }
    else
    {
        int entryIndex  = 0;
        while (entryPtr < freeSpace)
        {
            std::string key = restoreKey(entryPtr, buffer);
            if (key.size() > prefLen)
                key = key.substr(0, prefLen);
            int cmp = key.compare(prefix);
            if (cmp < 0)
            {
                entryPtr = nextEntry(entryPtr);
                ++entryIndex;
            }
            else if (cmp == 0)
            {
                result.push_back(entryID(entryPtr));
                owner.accessBlock(getChildIdx(entryIndex)).withPrefix(owner, prefix, prefLen, result);
                entryPtr = nextEntry(entryPtr);
                ++entryIndex;
            }
            else
            {
                owner.accessBlock(getChildIdx(entryIndex)).withPrefix(owner, prefix, prefLen, result);
                return;
            }
        }
        owner.accessBlock(getChildIdx(numberOfEntries())).withPrefix(owner, prefix, prefLen, result);
    }
}

int BtreeDict::ENTHEADERLEN = 6;
int BtreeDict::BLOCKSIZE = 2048;
int BtreeDict::DATALEN = BtreeDict::BLOCKSIZE - Block::HEADERLEN;
int BtreeDict::MaxKeyLength = 255;
  //!!! Careful with that number, Eugene
int BtreeDict::lastPtrIndex = 508;

DictBlock::DictBlock() : Block(BtreeDict::BLOCKSIZE)
{
}

int DictBlock::getChildIdx(int index) const
{
    return nthPointer(BtreeDict::lastPtrIndex - index);
}

int DictBlock::entryLength(int entry) const
{
    return BtreeDict::ENTHEADERLEN + entryKeyLength(entry);
}

int DictBlock::entryKey(int entry) const
{
    return entry + BtreeDict::ENTHEADERLEN;
}

void setBlockNumber2(std::vector<int> &blocks, size_t index, int number)
{
    if (index >= blocks.size())
        blocks.resize(index + 1000);
    blocks[index] = number;
}

class Entry
{
public:
    std::vector<unsigned char> key;
    int id;
    int block;

    Entry(const std::vector<unsigned char> &keyin, int length, int idin) : key(length+1), id(idin), block(-1)
    {
        memcpy(&key[0], &keyin[0], length);
    }

    Entry(const std::string &keyin, int idin) : key(keyin.size()+1), id(idin), block(-1)
    {
        memcpy(&key[0], keyin.c_str(), keyin.size());
    }

    bool smallerThan(const Entry &other)
    {
        for (size_t i = 0; i < std::min(key.size(), other.key.size()); i++)
            if (key[i] != other.key[i])
                return (key[i]&0xFF) < (other.key[i]&0xFF);
        return false;
    }
}; // end of internal class Entry

class FullDictBlock;

class FullBtreeDict : public BtreeDict
{
protected:
    BtreeDictParameters *_params;
    bool update;
public:
    FullBtreeDict(BtreeDictParameters &params, bool update);
    void store(const std::string &bla, int id) throw( HelpProcessingException );
    boost::shared_ptr<Entry> insert(FullDictBlock &bl, boost::shared_ptr<Entry> ent);
    boost::shared_ptr<Entry> insertHere(FullDictBlock &bl, boost::shared_ptr<Entry> ent)
         throw( HelpProcessingException );
    FullDictBlock& getNewBlock();
    void setModified(Block &bl);
    void close(int freeID);
};

class FullDictBlock : public DictBlock
{
public:
    virtual void setFree(int free);
    void setNumberOfEntries(int n) { setIntegerAt(0, n); }
    void setChildIndex(int index, int value)
    {
        setIntegerAt(4*(BtreeDict::lastPtrIndex - index + 1), value);
    }
    void setEntryID(int i, int id) { setIntegerAt(i + 2, id); }
    void setBlockNumbers(std::vector<int> &blocks) const;
    bool insert(const Entry &entry);
    void makeEntry(int entry, const std::vector<unsigned char> &key, int id, int length, int compr);
    bool insert(const Entry &ent, int entryPtr, int compr1, int compr2, int index);
    int insertInternal(const Entry &entry);
    boost::shared_ptr<Entry> split(FullDictBlock &newbl);
    void initInternal(int leftBlock, const Entry &entry);
    bool insert(boost::shared_ptr<Entry> entry);
    bool insert(boost::shared_ptr<Entry> ent, int entryPtr,
        int compr1, int compr2, int index);

};

void FullDictBlock::initInternal(int leftBlock, const Entry &entry)
{
    _isLeaf = false;
    setNumberOfEntries(1);
    setChildIndex(0, leftBlock);
    setChildIndex(1, entry.block);
    int ent = firstEntry();
    makeEntry(ent, entry.key, entry.id, entry.key.size() - 1, 0);
    setFree(nextEntry(ent));
}

void FullDictBlock::setFree(int infree)
{
    _free = infree - firstEntry();
    _data[infree] = _data[infree + 1] = 0; // sentinel
}

boost::shared_ptr<Entry> FullDictBlock::split(FullDictBlock& newbl)
{
    std::vector<unsigned char> buffer(BtreeDict::MaxKeyLength);
    int freeSpace = free();
    int half = freeSpace/2;
    int index = 0;          // of middle entry
    newbl._isLeaf = _isLeaf;
    int ent;
    for (ent = firstEntry(); ent < half; ent = nextEntry(ent))
    {
        restoreKeyInBuffer(ent, buffer);
        ++index;
    }
    int entriesToMove = numberOfEntries() - index - 1;
    // middle entry
    restoreKeyInBuffer(ent, buffer);
    int len = entryKeyLength(ent) + entryCompression(ent);
    boost::shared_ptr<Entry> result(new Entry(buffer, len, entryID(ent)));
    result->block = newbl._number;
    int newFree = ent;
    // rest goes to the new block
    ent = nextEntry(ent);
    restoreKeyInBuffer(ent, buffer);
    len = entryKeyLength(ent) + entryCompression(ent);
    int nptr = firstEntry();
    newbl.makeEntry(nptr, buffer, entryID(ent), len, 0);
    ent = nextEntry(ent);
    memmove(&(newbl._data[newbl.nextEntry(nptr)]), &(_data[ent]), freeSpace - ent);
    newbl.setNumberOfEntries(entriesToMove);
    newbl.setFree(newbl.nextEntry(nptr) + freeSpace - ent);
    if (_isLeaf == false) // need to split pointers
    {
        int from = 4*(BtreeDict::lastPtrIndex - numberOfEntries() + 1);
        int to = from + 4*(index + 1);
        memmove(&(newbl._data[to]), &(_data[from]), 4*(entriesToMove + 1));
    }
    // this entry will end here
    setFree(newFree);
    setNumberOfEntries(index);
    return result;
    //!!!remember updating ID -> string association
}

void FullDictBlock::setBlockNumbers(std::vector<int> &blocks) const
{
    for (int e = firstEntry(); e < _free; e = nextEntry(e))
        setBlockNumber2(blocks, entryID(e), _number);
}

bool FullDictBlock::insert(boost::shared_ptr<Entry> ent, int entryPtr,
    int compr1, int compr2, int index)
{
    const std::vector<unsigned char> &key = ent->key;
    int keyLen = key.size() - 1 - compr1;
    int freeSpace = free();
    // calculate how much space is needed to add the new entry
    // first, how many bytes are needed for just the new entry
    int demand = BtreeDict::ENTHEADERLEN + keyLen;
    // adding an entry can increase compression in the following entry

    int increase = 0;
    if (entryPtr < freeSpace)
        if (entryCompression(entryPtr) < compr2)
            increase = compr2 - entryCompression(entryPtr);
    /*
       std::cerr << "key " << key << std::endl;
       std::cerr << "entryPtr " << entryPtr << std::endl;
       std::cerr << "compr1 " << compr1) << std::endl;
       std::cerr << "compr2 " << compr2) << std::endl;
       std::cerr << "index " << index) << std::endl;
       std::cerr << "demand " << demand) << std::endl;
       std::cerr << "increase " << increase) << std::endl;
   */
   // check if enough space is available
    int limit = _isLeaf ? BtreeDict::DATALEN-2 : 4*(BtreeDict::lastPtrIndex-numberOfEntries()-1);

    if (freeSpace + demand - increase <= limit) // 2 for sentinel
    {
        if (entryPtr < freeSpace)
        {
            // need to shift extant entries forward
            int toMove = increase > 0 ? entryPtr + BtreeDict::ENTHEADERLEN + increase : entryPtr;
            // move entries
            memmove(&(_data[toMove + demand - increase]), &(_data[toMove]), freeSpace - toMove);

            if (increase > 0)
            {
                // update header
                unsigned char tmp = static_cast<unsigned char>(increase);
                _data[entryPtr] = _data[entryPtr] - tmp;
                _data[entryPtr + 1] = _data[entryPtr + 1] + tmp;
                // shift header
                memmove(&(_data[entryPtr + demand]), &(_data[entryPtr]), BtreeDict::ENTHEADERLEN);
            }
        }
        // now write the new entry in the space made above
        makeEntry(entryPtr, key, ent->id, keyLen, compr1);

        if (_isLeaf == false)
        {
            int from = 4*(BtreeDict::lastPtrIndex - numberOfEntries() + 1);
            memmove(&(_data[from - 4]), &(_data[from]), 4*(numberOfEntries() - index));
            setChildIndex(index + 1, ent->block);
        }
        setFree(freeSpace + demand - increase);
        setNumberOfEntries(numberOfEntries() + 1);

        /*
            System.err.println("------------list--------------");
            byte[] buffer = new byte[MaxKeyLength];
            final int freeSpace2 = free();
            int entryPtr2 = firstEntry();
            while (entryPtr2 < freeSpace2)
            {
                  System.err.println(entryPtr2);
                  System.err.println(entryKeyLength(entryPtr2));
                  System.err.println(entryCompression(entryPtr2));
                  System.err.println(new String(_data,
                  entryKey(entryPtr2),
                  entryKeyLength(entryPtr2)));
                  System.err.println(restoreKey(entryPtr2, buffer)+" "+
                  entryID(entryPtr2));
                  entryPtr2 = nextEntry(entryPtr2);
            }
            System.err.println("------------end--------------");
        */
        return true;
    }
    else
        return false;
}

// finds the place and context
bool FullDictBlock::insert(boost::shared_ptr<Entry> entry)
{
    const std::vector<unsigned char> &inkey = entry->key;
    int inputKeyLen = inkey.size() - 1;
    int freeSpace  = free();
    int entryPtr    = firstEntry();
    int nCharsEqual = 0;
    int prevNCEqual = 0;
    int compression = 0;

    for (int entryIndex = 0;;)
    {
        if (entryPtr == freeSpace)
            return insert(entry, entryPtr, nCharsEqual, 0, numberOfEntries());
        else if (compression == nCharsEqual)
        {
            int keyLen = entryKeyLength(entryPtr);
            int keyPtr = entryKey(entryPtr), i;
            prevNCEqual = nCharsEqual;
            for (i = 0; i < keyLen && inkey[nCharsEqual] == _data[keyPtr + i]; i++)
                ++nCharsEqual;
            if (i == keyLen)
            {
                if (nCharsEqual == inputKeyLen)
                {
                    HCDBG(std::cerr << "setting to " << entry->id << std::endl);
                    setEntryID(entryPtr, entry->id);
                    return true;
                }
            }
            else if ((inkey[nCharsEqual]&0xFF) < (_data[keyPtr + i]&0xFF))
                return insert(entry, entryPtr, prevNCEqual, nCharsEqual, entryIndex);
        }
        else if (compression < nCharsEqual) // compression dropped
        {
            int index = entryPtr == freeSpace ? numberOfEntries() : entryIndex;
            return insert(entry, entryPtr, nCharsEqual, compression, index);
        }
        do
        {
            entryPtr = nextEntry(entryPtr);
            ++entryIndex;
        }
        while (entryCompression(entryPtr) > nCharsEqual);
        compression = entryCompression(entryPtr);
    }
}

static int fulldictcount;

class FullDictBlockFactory : public BlockFactory
{
public:
    Block* makeBlock() const
    {
        fulldictcount++;
        return new FullDictBlock;
    }
};

class FullDictBlockProcessor : public BlockProcessor
{
public:
    FullDictBlockProcessor(std::vector<int> &_blocks) : BlockProcessor(_blocks) {}
    void process(const Block &block)
    {
        ((const FullDictBlock&)block).setBlockNumbers(blocks);
    }
};

FullBtreeDict::FullBtreeDict(BtreeDictParameters &params, bool _update) :
    _params(&params), update(_update)
{
    init(_params, update, new FullDictBlockFactory());
    HCDBG(std::cerr << "id is " << params.getFreeID() << std::endl);
    blocks.resize(params.getFreeID());

    FullDictBlockProcessor foo(blocks);
    blockManager->processBlocks(foo);
    /*
    if (logging)
        log = new FileWriter("/tmp/FullBtreeDict.log");
    */
}

void FullBtreeDict::setModified(Block &bl)
{
    blockManager->setModified(bl._number);
}

FullDictBlock& FullBtreeDict::getNewBlock()
{
    FullDictBlock &nbl = (FullDictBlock&)blockManager->getNewBlock();
    setModified(nbl);
    return nbl;
}

boost::shared_ptr<Entry> FullBtreeDict::insertHere(FullDictBlock &bl, boost::shared_ptr<Entry> ent)
    throw( HelpProcessingException )
{
    setModified(bl);                // to be modified in any case
    if (bl.insert(ent))
        return boost::shared_ptr<Entry>();
    else
    {
        FullDictBlock &nbl = getNewBlock();
        boost::shared_ptr<Entry> middle = bl.split(nbl);
        nbl.setBlockNumbers(blocks);
        if ((middle->smallerThan(*ent) ? nbl : bl).insert(ent) == false)
        {
            std::stringstream aStrStream;
            aStrStream << "entry didn't fit into a freshly split block" << std::endl;
            throw HelpProcessingException( HELPPROCESSING_INTERNAL_ERROR, aStrStream.str() );
        }
        return middle;
    }
}

void FullDictBlock::makeEntry(int entry, const std::vector<unsigned char> &key, int id, int length, int compr)
{
    _data[entry] = static_cast<unsigned char>(length);
    _data[entry + 1] = static_cast<unsigned char>(compr);
    setEntryID(entry, id);
    memmove(&(_data[entryKey(entry)]), &(key[compr]), length);
}

int FullDictBlock::insertInternal(const Entry &entry)
{
    const std::vector<unsigned char> &inkey = entry.key;
    int inputKeyLen = inkey.size() - 1;
    int entryPtr = firstEntry();
    int freeSpace = free();
    int nCharsEqual = 0;
    int compression = 0;

    for (int entryIndex = 0;;)
    {
        if (entryPtr == freeSpace)
            return numberOfEntries();
        else if (compression == nCharsEqual)
        {
            int i;
            int keyLen = entryKeyLength(entryPtr);
            int keyPtr = entryKey(entryPtr);
            for (i = 0; i < keyLen && inkey[nCharsEqual] == _data[keyPtr + i]; i++)
                ++nCharsEqual;
            if (i == keyLen)
            {
                if (nCharsEqual == inputKeyLen)
                {
                    setEntryID(entryPtr, entry.id);
                    return -1;
                }
            }
            else if ((inkey[nCharsEqual]&0xFF) < (_data[keyPtr + i]&0xFF))
                return entryIndex;
        }
        else if (compression < nCharsEqual) // compression dropped
            return entryPtr >= freeSpace ? numberOfEntries() : entryIndex;

        do
        {
            entryPtr = nextEntry(entryPtr);
            ++entryIndex;
        }
        while (entryCompression(entryPtr) > nCharsEqual);
            compression = entryCompression(entryPtr);
    }
}

/*
  delegation to powerful primitives at the FullDictBlock level lets us
  express the insertion algorithm very succintly here
*/
boost::shared_ptr<Entry> FullBtreeDict::insert(FullDictBlock &bl, boost::shared_ptr<Entry> ent)
{
    if (bl._isLeaf)
        ent = insertHere(bl, ent);
    else
    {
        int index = bl.insertInternal(*ent);
        if (index != -1)
        {
            ent = insert((FullDictBlock&)child(bl, index), ent);
            if (ent.get())
                ent = insertHere(bl, ent);
        }
    }
    return ent;
}

void FullBtreeDict::store(const std::string &key, int id) throw( HelpProcessingException )
{
    HCDBG(std::cerr << "so storing " << key << " id " << id << std::endl);

    if (key.size() >= 250)
    {
        std::stringstream aStrStream;
        aStrStream << "token " << key << " too long" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_INTERNAL_ERROR, aStrStream.str() );
    }
    boost::shared_ptr<Entry> aTemp(new Entry(key, id));
    FullDictBlock &rBlock = (FullDictBlock&)accessBlock(root);
    boost::shared_ptr<Entry> entry = insert(rBlock, aTemp);
    if (entry.get())
    {
        // new root; writing to params needed
        FullDictBlock &nbl = getNewBlock();
        nbl.initInternal(root, *entry);
        setBlockNumber2(blocks, entry->id, root = nbl._number);
        _params->setRoot(root);
    }
}

void FullBtreeDict::close(int freeID)
{
    _params->setFreeID(freeID);
    if (update)
        _params->updateSchema();
    BtreeDict::close();
    /*
    if (logging)
        log.close();
    */
}

class ConceptLocation
{
public:
    int _concept;
    int _begin;
    int _end;
public:
    ConceptLocation(int conceptID, int begin, int end);
    static void sortByConcept(std::vector<ConceptLocation> &array, int i1, int i2);
    static void sortByPosition(std::vector<ConceptLocation> &array, int i1, int i2);
    int getConcept() const { return _concept; }
    void setConcept(int concept) { _concept = concept; }
    int getBegin() const { return _begin; }
    int getEnd() const { return _end; }
    int getLength() const { return _end - _begin; }
    bool equals(const ConceptLocation &other) const
    {
        return _concept==other._concept&&_begin==other._begin&&_end==other._end;
    }
};

class DocumentCompressor;

class Index : public IndexAccessor
{
protected:
    typedef std::hash_map<std::string, int, pref_hash> IndexHashtable;
    bool _update;
    IndexHashtable _cache;
    Schema *_schema;
private:
    BtreeDictParameters *_dictParams;
    FullBtreeDict *_dict;
    int _freeID;
    std::fstream *_positionsFile;
    std::fstream *_offsetsFile;
    DocumentCompressor *_documentCompressor;
    IntegerArray _concepts;
    IntegerArray _offsets;
    std::vector<unsigned char> _allLists; // POSITIONS
    void readDocumentsTable(const std::string &fileName);
    void readOffsetsTables(const std::string &fileName);
    void readPositions();
protected:
    IntegerArray _microIndexOffsets;
    IntegerArray _documents;
    IntegerArray _titles;
    std::vector<unsigned char>  _positions;
private:
    int _positionsCacheSize;
    int _currentBatchOffset;
    bool _allInCache;
protected:
    virtual void writeOutOffsets();
public:
    Index(const fs::path &indexName, bool update);
    virtual ~Index();
    void init();
    int intern(const std::string &name);
    std::fstream& getPositionsFile();
    std::fstream& getOffsetsFile();
    DocumentCompressor& getDocumentCompressor();
    virtual void compress(int docID, int titleID,
        std::vector<ConceptLocation> &locations,
        std::vector<ConceptLocation> &extents);
    void close();
};

Index::Index(const fs::path &indexName, bool update) : IndexAccessor(indexName),
    _update(update), _cache(256), _schema(NULL), _dictParams(NULL), _dict(NULL), _positionsFile(0), _offsetsFile(0), _documentCompressor(0),
    _positionsCacheSize(0), _currentBatchOffset(0), _allInCache(false)
{
}

class CompressorIterator;
class Decompressor
{
private:
    static int BitsInByte;
    static int NBits;

    int _readByte;
    int _toRead;
    int _path;

protected:
    virtual int getNextByte() = 0;
    virtual void initReading() { _toRead = 0; _path = 0; }

private:
    int countZeroes();
    // reads 1 bit; returns non-0 for bit "1"
    int read();

public:
    int read(int kBits);
    void beginIteration() { _path = 0; }
    bool readNext(int k, CompressorIterator &it);
    void decode(int k, IntegerArray &array);
    void ascDecode(int k, IntegerArray &array);
    int ascendingDecode(int k, int start, std::vector<int> &array);
    virtual ~Decompressor() {}
};

int Decompressor::BitsInByte = 8;
int Decompressor::NBits = 32;

class ByteArrayDecompressor : public Decompressor
{
private:
    const std::vector<unsigned char> *_array;
    int _index;
    int _index0;
public:
    ByteArrayDecompressor(const std::vector<unsigned char> *array, int index) { initReading(array, index); }
    using Decompressor::initReading;
    virtual void initReading(const std::vector<unsigned char> *array, int index)
    {
        _array = array;
        _index = _index0 = index;
        Decompressor::initReading();
    }
    int bytesRead() { return _index - _index0; }
protected:
    int getNextByte()
    {
        int ret = (*_array)[_index] & 0xFF;
        HCDBG(fprintf(stderr, "ByteArrayDecompressor::getNextByte of %d at index %d\n", ret, _index));
        _index++;
        return ret;
    }
};

bool isExtensionMode( void );

class IndexInverter;

class MicroIndex
{
public:
    static int RANGE;
    static int NConcepts;
private:
    int _currentRange;
    int _documentNumber;
    std::vector<int> _concepts;
    short _group;
    short _ix;
    IntegerArray _kTable;
    IntegerArray _offsets;
    IntegerArray _maxConcepts;
    const std::vector<unsigned char> *_data;
    int _base;
    int _limit;
    int _nc;
    ByteArrayDecompressor _decmp;
public:
    MicroIndex(int documentNumber, const std::vector<unsigned char> *positions, int index);
    bool smallerThan(const MicroIndex &other)
    {
        return _currentRange < other._currentRange ||
            _currentRange == other._currentRange &&
            _documentNumber < other._documentNumber;
    }

private:
    bool next()
    {
        if (_group <= _limit)
        {
            int shift, index;
            if (_group > 0)
            {
                index = _base + _offsets[_group - 1];
                shift = _maxConcepts[_group - 1];
            }
            else
            {
                index = _base;
                shift = 0;
            }

            _decmp.initReading(_data, index);
            _nc = _decmp.ascendingDecode(_kTable[_group*2], shift, _concepts);
            HCDBG(std::cerr << "nc b set to " << _nc << std::endl);
            if (_group < _limit)
            {
                HCDBG(fprintf(stderr, "microindex concept index %d set to %d\n", _nc, _maxConcepts[_group]));
                _concepts[_nc++] = _maxConcepts[_group];
            }
            _currentRange = _concepts[_ix = 0]/RANGE;
            _group++;
            return true;
        }
        else
            return false;
    }

    void openDocumentIndex()
    {
        unsigned int kk = (*_data)[_base] & 0xFF;
        HCDBG(std::cerr << "openDocumentIndex, kk is " << kk
            << " base is " << _base << std::endl);
        switch (kk >> 6)    // get type
        {
            case 0:         // single group, no extents
                _decmp.initReading(_data, _base += 2);
                _nc = _decmp.ascendingDecode(kk & 0x3F, 0, _concepts);
                HCDBG(std::cerr << "nc a set to " << _nc << std::endl);
                _currentRange = _concepts[_ix = 0]/RANGE;
                _limit = 0;
                _group = 1;
                break;
            case 2:         // multi group, no extents
            {
                _decmp.initReading(_data, _base + 1);
                _decmp.decode(kk & 0x3F, _kTable);
                int last = _kTable.back();
                _kTable.pop_back();
                _decmp.ascDecode(last, _offsets);
                last = _kTable.back();
                _kTable.pop_back();
                _decmp.ascDecode(last, _maxConcepts);
                _base += 1 + _decmp.bytesRead();
                _limit = _maxConcepts.size();
                _group = 0;
                next();
            }
                break;
            case 1:         // single group, extents
            case 3:         // multi group, extents
                if( !isExtensionMode() )
                    std::cerr << "extents not yet implemented" << std::endl;
                break;
        }
    }

public:
    bool process(IndexInverter &lists);
};

int MicroIndex::RANGE = 1024;
int MicroIndex::NConcepts = 16;

class BitBuffer
{
private:
    static int InitSize;
    static int NBits;
    static int BitsInByte;
    static int BytesInInt;

    int _avail;
    unsigned int _word;
    int _free;
    int _size;
    std::vector<unsigned int> _array;

public:
    BitBuffer() : _avail(NBits), _word(0), _free(0), _size(InitSize)
    {
        _array.resize(InitSize);
    }

    void close()
    {
        if (_avail < NBits)
            store(_word << _avail);
        else
            _avail = 0;
    }

    void write(std::fstream &out) const
    {
        for (int i = 0; i < _free - 1; i++)
            writeInt(out, _array[i]);
        unsigned int word = _array[_free - 1];
        int bytes = BytesInInt - _avail/BitsInByte;
        int shift = NBits;
        while (bytes-- > 0)
            writeByte(out, static_cast<unsigned char>((word >> (shift -= BitsInByte)) & 0xFF));
    }

    void clear()
    {
        _word  = 0;
        _avail = NBits;
        _free  = 0;
    }

    int byteCount() { return _free*BytesInInt - _avail/BitsInByte; }
    int bitCount() { return _free*NBits - _avail; }

    void setFrom(const BitBuffer &rhs)
    {
        _word  = rhs._word;
        _avail = rhs._avail;
        if ((_free = rhs._free) > _size)
            _array.resize(_size = rhs._free);
        _array = rhs._array;
    }
private:
    void growArray(int newSize)
    {
        _array.resize(newSize);
        _size = newSize;
    }

    void store(unsigned int value)
    {
        if (_free == _size)
            growArray(_size * 2);
        HCDBG(fprintf(stderr, "store of %x to %d\n", (int)value, _free));
        _array[_free++] = value;
    }

public:
    void append(int bit)
    {
        _word = (_word << 1) | bit;
        if (--_avail == 0)
        {
            store(_word);
            _word = 0;
            _avail = NBits;
        }
    }

    void append(unsigned int source, int kBits)
    {
        if (kBits < _avail)
        {
            _word = (_word << kBits) | source;
            _avail -= kBits;
        }
        else if (kBits > _avail)
        {
            int leftover = kBits - _avail;
            store((_word << _avail) | (source >> leftover));
            _word = source;
            _avail = NBits - leftover;
        }
        else
        {
            store((_word << kBits) | source);
            _word = 0;
            _avail = NBits;
        }
    }

    void concatenate(const BitBuffer &bb)
    {
        if (_size - _free < bb._free)
            growArray(_free + bb._free + 1);

        if (_avail == 0)
        {
            memmove(&_array[_free], &bb._array[0], bb._free * sizeof(unsigned int));
            _avail = bb._avail;
            _free += bb._free;
            HCDBG(fprintf(stderr, "free bumped to %d\n", _free));
        }
        else
        {
            int tp = _free - 1; // target
            int sp = 0;     // source
            do
            {
                _array[tp] |= bb._array[sp] >> (NBits - _avail);
                _array[++tp] = bb._array[sp++] << _avail;
            }
            while (sp < bb._free);
            _free += bb._free;
            if ((_avail += bb._avail) >= NBits)
            {
                _avail -= NBits;
                _free--;
            }
            HCDBG(fprintf(stderr, "other free bumped to %d\n", _free));
        }
    }
};

class Compressor
{
private:
    static int NBits;
    static int BeginK;
    BitBuffer _buffer;
public:
    void write(std::fstream &out) const { _buffer.write(out); }
    int byteCount() { return _buffer.byteCount(); }
    void clear() { _buffer.clear(); }
    void concatenate(const Compressor &other) { _buffer.concatenate(other._buffer); }
    void encode(const IntegerArray &pos, int k);
    void encode(const IntegerArray &pos, const IntegerArray &len, int k, int k2);
    // k: starting value for minimization
    int minimize(const IntegerArray &array, int startK);
    int compressAscending(const IntegerArray &array);
};

void toDifferences(const IntegerArray &in, IntegerArray &out)
{
    if (out.size() < in.size())
        out.resize(in.size());
    if (in.empty())
        return;
    out[0] = in[0];
    for (size_t i = 1; i < in.size(); ++i)
        out[i] = in[i] - in[i - 1];
}

class IndexInverter
{
private:
    static int K;
    std::vector<IntegerArray> _arrays;
    int _minConcept;
    int _limit;
    IntegerArray _concepts;
    IntegerArray _offsets;
    Compressor _compr;
    IntegerArray _diffs;
    std::fstream *_mainFile;
    // heap
    int _heapSize;
    std::vector<MicroIndex*> _heap;

    Index &_index;

public:
    IndexInverter(Index &index) : _arrays(MicroIndex::RANGE),
        _minConcept(0), _limit(MicroIndex::RANGE),
        _mainFile(0), _heapSize(0), _index(index) {}
    ~IndexInverter()
    {
        delete _mainFile;
        for (int i = 0; i < _heapSize; i++)
        {
            HCDBG(fprintf(stderr, "deleting number %d\n", i));
            delete _heap[i];
        }
    }
    void invertIndex(int nDocuments, const IntegerArray &microIndexOffsets)
    {
        _mainFile = _index.getOutputStream("DOCS");
        for (int i = 0; i < MicroIndex::RANGE; i++)
            _arrays[i] = IntegerArray();

        // read in the whole POSITIONS file
        std::vector<unsigned char> positions = _index.readByteArray("POSITIONS");
        // build heap
        _heap.clear();
        _heap.resize(_heapSize = nDocuments);
        for (int i = 0; i < nDocuments; i++)
            _heap[i] = new MicroIndex(i, &positions, microIndexOffsets[i]);
        for (int i = _heapSize/2; i >= 0; i--)
            heapify(i);
        // process till exhausted
        while (!_heap.empty())
            if (_heap[0]->process(*this))
                heapify(0);
            else if (_heapSize > 1)
            {
                delete _heap[0];
                _heap[0] = _heap[--_heapSize];
                heapify(0);
            }
            else
                break;
        // closing
        flush();
        _mainFile->close();
        // compress index file
        std::fstream *indexFile = _index.getOutputStream("DOCS.TAB");
        unsigned char byte = static_cast<unsigned char>(
            _compr.compressAscending(_concepts));
        indexFile->write( (const char*)&byte, 1 ); // write k
        _compr.write(*indexFile);
        _compr.clear();
        byte = static_cast<unsigned char>(_compr.minimize(_offsets, K));
        indexFile->write( (const char*)&byte, 1 ); // write k
        _compr.write(*indexFile);
        indexFile->close();
        delete indexFile;
    }

    short process(int documentNumber, std::vector<int> &concepts,
               int n, short start, bool firstTime)
    {
        if (firstTime && concepts[start] >= _limit)
            flush();
        concepts[n] = _limit; // sentinel
        while (concepts[start] < _limit)
        {
            _arrays[concepts[start++] - _minConcept].push_back(documentNumber);
        }
        return start;
    }

private:
    void heapify(int i)
    {
        int r = (i + 1) << 1, l = r - 1;
        int smallest = l < _heapSize && _heap[l]->smallerThan(*_heap[i]) ? l : i;
        if (r < _heapSize && _heap[r]->smallerThan(*_heap[smallest]))
            smallest = r;
        if (smallest != i)
        {
            MicroIndex *temp = _heap[smallest];
            _heap[smallest] = _heap[i];
            _heap[i] = temp;
            heapify(smallest);
        }
    }

    void flush()
    {
        for (int i = 0; i < MicroIndex::RANGE; ++i)
        {
            if (!_arrays[i].empty())
            {
                toDifferences(_arrays[i], _diffs);
                unsigned char byte = static_cast<unsigned char>(
                    _compr.minimize(_diffs, K));
                _mainFile->write( (const char*)&byte, 1 ); // write k
                _offsets.push_back(_compr.byteCount() + 1);
                _compr.write(*_mainFile);
                _concepts.push_back(_minConcept + i);
                _arrays[i].clear();
                _diffs.clear();
                _compr.clear();
            }
        }
        _limit += MicroIndex::RANGE;
        _minConcept += MicroIndex::RANGE;
    }
};

int IndexInverter::K = 3;

MicroIndex::MicroIndex(int documentNumber, const std::vector<unsigned char> *positions, int index)
    : _concepts(NConcepts + 1), _data(positions), _decmp(NULL, 0)
{
    _documentNumber = documentNumber;
    _base = index;
    openDocumentIndex();
}

bool MicroIndex::process(IndexInverter &lists)
{
    bool firstTime = true;
    while (true)
    {
        short stop = lists.process(_documentNumber, _concepts, _nc, _ix, firstTime);
        if (stop < _nc)
        {
            _currentRange = _concepts[_ix = stop]/RANGE;
            return true;
        }
        else if (next())
            firstTime = false;
        else
            return false;
    }
}

void Index::close()
{
    /*
      BtreeDictCompactor source = new BtreeDictCompactor(_dictParams, false);

      URL url = new URL("file", "", _indexDir + "compacted");
      BtreeDictParameters params =
      new BtreeDictParameters(url, _dictParams.getBlockSize(), 0, _freeID);
      source.compact(params);
      URL tmapURL = new URL("file", "", _indexDir + "DICTIONARY");
      File tmap = new File(tmapURL.getFile());
      File compacted = new File(url.getFile());
      compacted.renameTo(tmap);
      _dictParams.setRoot(params.getRootPosition());
      _dictParams.updateSchema();
      */
    _dict->close(_freeID);
    if (_positionsFile)
    {
        delete _positionsFile;
        _positionsFile = NULL;
    }

    if (_update)
    {
        writeOutOffsets();
        _dictParams->setFreeID(_freeID);
        _dictParams->updateSchema();
        _schema->save();
        IndexInverter inverter(*this);
        inverter.invertIndex(_documents.size(), _microIndexOffsets);
    }
    if (_offsetsFile)
    {
        delete _offsetsFile;
        _offsetsFile = NULL;
    }
}

void Index::init()
{
    bool indexExists = false;
    if (_update)
    {
        createIfNeeded();
        _cache.clear();
    }
    if (_schema) delete _schema;
    _schema = new Schema(*this, _update);

    if (_dictParams) delete _dictParams;
    _dictParams = new BtreeDictParameters(*_schema, "DICTIONARY");

    if (_dictParams->readState() == false)
    {
        _dictParams->setBlockSize(2048);
        _dictParams->setRoot(0);
        _dictParams->setFreeID(1);
    }
    else
        indexExists = true;

    if (_dict) delete _dict;
    _dict = new FullBtreeDict(*_dictParams, _update);

    _freeID = _dictParams->getFreeID();

    _documents.clear();
    if (indexExists)
    {
        // read in index parts
        _allLists = readByteArray("DOCS");
        readDocumentsTable("DOCS.TAB");
        readOffsetsTables("OFFSETS");
        readPositions();
    }
    else
    {
        _microIndexOffsets.clear();
        _titles.clear();
    }
}

namespace
{
    std::string cliptoken(const std::string &name)
    {
        std::string key = name;
        int length = key.size();
        while(key.size() >= 250)
            key = name.substr(--length);
       return key;
    }
}

int Index::intern(const std::string &name)
{
    std::string key = cliptoken(name);
    IndexHashtable::const_iterator aIter = _cache.find(key);
    if (aIter != _cache.end())
        return aIter->second;
    else
    {
        //Seeing as we always start off with an empty dictionary,
        //our entries will always be in the _cache, so don't ever
        //search the underlying dictionary
        int id = _freeID++;
        _dict->store(key, id);
        _cache.insert(IndexHashtable::value_type(key, id)).first->second = id;
        return id;
    }
}

std::fstream& Index::getPositionsFile()
{
    if (!_positionsFile)
        _positionsFile = getRAF("POSITIONS", _update);
    return *_positionsFile;
}

std::fstream& Index::getOffsetsFile()
{
    if (!_offsetsFile)
        _offsetsFile = getRAF("OFFSETS", _update);
    return *_offsetsFile;
}

class VectorBtreeParameters : public BlockManagerParameters
{
private:
    int _vectorLength;
public:
    VectorBtreeParameters(Schema &schema, const std::string &partName) :
        BlockManagerParameters(schema, partName)
    {
        _vectorLength = integerParameter("vl");
    }

    void updateSchema()
    {
        std::ostringstream tmp;
        tmp << "vl=" << _vectorLength;
        BlockManagerParameters::updateSchema(tmp.str());
    }

    VectorBtreeParameters(Schema &schema, const std::string &partName, int vecLen)
        : BlockManagerParameters(schema, partName)
    {
        _vectorLength = vecLen;
    }

    int getVectorLength() { return _vectorLength; }
};

enum outerbreak { dobreak, docontinue, donothing };

class VectorProcessor
{
    std::vector<unsigned char> _vector;
public:
    virtual bool processVector() = 0;
    std::vector<unsigned char>& getVectorBuffer() { return _vector; }
    virtual ~VectorProcessor() {}
};

class VectorBlock;

class VectorBtree
{
protected:
    VectorBlock *_root;
    BlockManager *_blockManager;
    VectorBtreeParameters *_params;
    int _blockSize;
public:
    int _maxEntries;
    int _leafDataLimit;
protected:
    int _vectorsOffset;
    VectorBlock& accessBlock(int index);
    VectorBtree() {/*empty*/}
public:
    int _vecLen;
    int vector(int index) const;
    static int memcmp(const std::vector<unsigned char> &v1,
        const std::vector<unsigned char> &v2, int i2, int n);
    VectorBtree(VectorBtreeParameters *params);
    ~VectorBtree() { delete _blockManager; }
};

class VectorBlockFactory : public BlockFactory
{
private:
    int _blockSize;
public:
    VectorBlockFactory(int blockSize) : _blockSize(blockSize) {}
    Block* makeBlock() const;
};

VectorBtree::VectorBtree(VectorBtreeParameters *params)
{
    _params = params;
    _vecLen = params->getVectorLength();
    _blockSize = params->getBlockSize();
    _maxEntries=(_blockSize-Block::HEADERLEN-Block::IDLEN)/(_vecLen+Block::IDLEN);
    if ((_maxEntries & 1) == 0) // needs to be odd
        _maxEntries--;

    _leafDataLimit = _blockSize - _vecLen -  Block::HEADERLEN - Block::IDLEN;

    _vectorsOffset = (_maxEntries + 1)*Block::IDLEN;
    _blockManager = new BlockManager(_params, false, new VectorBlockFactory(_blockSize));
    _root = &(accessBlock(params->getRootPosition()));
}

VectorBlock& VectorBtree::accessBlock(int index)
{
    return (VectorBlock&)_blockManager->accessBlock(index);
}

int VectorBtree::memcmp(const std::vector<unsigned char> &v1,
        const std::vector<unsigned char> &v2, int i2, int n)
{
    for (int i = 0; i < n; i++, i2++)
        if (v1[i] != v2[i2])
            return (v1[i]&0xFF) - (v2[i2]&0xFF);
    return 0;
}

class VectorBlock : public Block
{
public:
    VectorBlock(int size) : Block(size) {}
protected:
    int findIndex(const std::vector<unsigned char> &key, const VectorBtree &tree)
    {
        int i = 0, j = _free - 1;
        while (i <= j)
        {
            int k = (i + j)/2;
            int test = VectorBtree::memcmp(key, _data, tree.vector(k),tree._vecLen);
            //      std::cerr << "k = " << k << ", test = " << test << std::endl;
            if (test > 0)
                i = k + 1;
            else if (test < 0)
                j = k - 1;
            else
                return -1 - k; // result always negative; "k" encoded
        }
        return i;
    }
private:
    int FindVectorsInLeaf(const std::vector<unsigned char> &lo,
        const std::vector<unsigned char> &hi, int commLen, int prefLen,
        std::vector<unsigned char> &buffer, int size, const VectorBtree &tree)
    {
        int idx = 0, start;
        for (int nBytesEq = 0;;)
        {
            //      std::cout << "idx = " << idx << std::endl;
            if (_data[idx] == nBytesEq) // at compression byte
            {
                int i;
                outerbreak hack(donothing);
                for (i = nBytesEq; i < tree._vecLen; i++)
                {
                    if (lo[i] == _data[++idx])
                        ++nBytesEq;
                    else if ((lo[i]&0xFF) < (_data[idx]&0xFF))
                        if (nBytesEq >= commLen && (i >= prefLen || (hi[i]&0xFF) >= (_data[idx]&0xFF)))
                        {
                            start = nBytesEq;
                            hack = dobreak;
                            break;
                        }
                        else
                            return 0;
                    else
                    {
                        idx += tree._vecLen - i; // skip
                        hack = docontinue;
                        break;
                    }
                }

                if (hack == dobreak)
                    break;
                else if (hack == docontinue)
                    continue;

                if (i == tree._vecLen)     // eq vec found
                    if ((_data[++idx]&0xFF) >= prefLen)
                    {
                        start = _data[idx++]&0xFF;
                        break;
                    }
                    else
                        return 0;
            }
            else if (_data[idx] < nBytesEq) // drop
            {
                std::cout << idx << std::endl;
                nBytesEq = (_data[idx++]);
                std::cout << nBytesEq << std::endl;
                if (nBytesEq < commLen)
                    return 0;
                else if (lo[nBytesEq] < (_data[idx]&0xFF))
                    if (hi[nBytesEq] < (_data[idx]&0xFF))
                        return 0;
                    else
                    {
                        start = nBytesEq;                 // found
                        break;
                    }
                else
                    idx += tree._vecLen - nBytesEq;
            }
            else if ((_data[idx]&0xFF) == 0xFF)
                return 0;
            else                      // compression is bigger
                idx += tree._vecLen + 1 - _data[idx];
        }

        int length = std::min(size - start, _free - idx);
        buffer[0] = static_cast<unsigned char>(start);
        memcpy(&(buffer[1]), &(_data[idx]), length);
        buffer[length + 1] = 0;
        return length + 1;
    }
protected:
    bool searchLeafBlock(const std::vector<unsigned char> &key, const VectorBtree &tree)
    {
#if 0
        processLeafBlock(_printer);
#endif
        int nBytesEq = 0;
        for (int idx = 0;; idx += tree._vecLen + 1 - _data[idx])
        {
            if (_data[idx] == nBytesEq)
            {
                int i, j;
                outerbreak hack(donothing);
                for (i = _data[idx], j = idx + 1; i < tree._vecLen; i++, j++)
                {
                    if (key[i] == _data[j])
                        ++nBytesEq;
                    else if ((key[i]&0xFF) < (_data[j]&0xFF))
                        return false;
                    else                  /* key[i] > _data[j] */
                    {
                        hack = dobreak;
                        break;
                    }
                }

                if (hack == dobreak)
                    break;

                if (i == tree._vecLen)               /* or nBytesEq == _vecLen */
                    return true;          /* equal vector found */
            }
            else if (_data[idx] < nBytesEq)
                return false;
        }
        return false;
    }
public:
    bool processLeafBlock(VectorProcessor &processor, const VectorBtree &tree)
    {
        std::vector<unsigned char> &buffer = processor.getVectorBuffer();
        for (int ix = 0; ix < _free; ix += tree._vecLen - _data[ix] + 1)
        {
        //      cmc: the below line was a comment in the original java, somewhere along
        //      the line I suspect this was written in c++, then into java
        //      and now I'm putting it back to c++ :-(
        //      ::memcpy(&buffer[_data[ix]], &_data[ix + 1], _vecLen - _data[ix]);
            memcpy(&(buffer[_data[ix]]), &(_data[ix + 1]), tree._vecLen - _data[ix]);
            if (processor.processVector())
                return true;
        }
        return false;
    }
}; // VectorBlock

Block* VectorBlockFactory::makeBlock() const
{
    return new VectorBlock(_blockSize);
}

class FullVectorBlock : public VectorBlock
{
public:
    FullVectorBlock(int size) : VectorBlock(size) {}
    bool isFull(const VectorBtree &tree) const
    {
      //return pbl->_leaf ? pbl->_free > _leafDataLimit : pbl->_free == _maxEntries;
      return _isLeaf ? _free > tree._leafDataLimit : _free == tree._maxEntries;
    }
};

class FullVectorBtree : public VectorBtree
{
private:
    static int MaxVeclen;
    static double SplitRatio;
public:
    FullVectorBtree(VectorBtreeParameters* params, bool update);
    bool insertVector(const std::vector<unsigned char> &key);
private:
    bool treeInsertNonfull(const FullVectorBlock &bl, const std::vector<unsigned char> &key);
    bool treeInsertNonfullRoot(const std::vector<unsigned char> &key);
    FullVectorBlock& getNewBlock();
    void enableModif(const Block &bl);
    void declareModif(const Block &bl);
public:
    void close() { _blockManager->close(); }
};

int FullVectorBtree::MaxVeclen = 128;
double FullVectorBtree::SplitRatio = 0.5;

class FullVectorBlockFactory : public BlockFactory
{
private:
    int _blockSize;
public:
    FullVectorBlockFactory(int blockSize) : _blockSize(blockSize) {}
    Block* makeBlock() const
    {
        return new FullVectorBlock(_blockSize);
    }
};

FullVectorBtree::FullVectorBtree(VectorBtreeParameters *params, bool update)
{
    _params = params;
    _vecLen = params->getVectorLength();
    _blockSize = params->getBlockSize();
    _blockManager = new BlockManager(params, update, new FullVectorBlockFactory(_blockSize));
    _maxEntries=(_blockSize-Block::HEADERLEN-Block::IDLEN)/(_vecLen+Block::IDLEN);
    // System.out.println("_maxEntries = " + _maxEntries);
    if ((_maxEntries & 1) == 0)       // needs to be odd
        _maxEntries--;
    _leafDataLimit = _blockSize - _vecLen -  Block::HEADERLEN - Block::IDLEN;
    _vectorsOffset = (_maxEntries + 1)*Block::IDLEN;
    _root = &(accessBlock(params->getRootPosition()));
}

class CompressorIterator
{
public:
    virtual void value(int value) = 0;
    virtual ~CompressorIterator() {}
};

int Decompressor::countZeroes()
{
    for (int count = 0;; _readByte = getNextByte(), _toRead = BitsInByte)
    {
        HCDBG(fprintf(stderr, "count is %d\n", count));
        HCDBG(fprintf(stderr, "Decompressor::countZeroes is %x\n", _readByte));
        HCDBG(fprintf(stderr, "_toRead is %d\n", _toRead));
        HCDBG(fprintf(stderr, "_readByte is %x\n", _readByte));
        while (_toRead-- > 0)
        {
            if ((_readByte & (1 << _toRead)) != 0)
            {
                HCDBG(fprintf(stderr, "returning count of %d\n", count));
                return count;
            }
            else
            {
                ++count;
                HCDBG(fprintf(stderr, "int count to %d\n", count));
            }
        }
    }
    //return 0;
}

// reads 1 bit; returns non-0 for bit "1"
int Decompressor::read()
{
    if (_toRead-- > 0)
        return _readByte & (1 << _toRead);
    else
    {  // get next word
        _toRead = BitsInByte - 1;
        return (_readByte = getNextByte()) & 0x80;
    }
}

int Decompressor::read(int kBits)
{
    int shift = BitsInByte - _toRead;
    if (kBits <= _toRead)
    {
        HCDBG(fprintf(stderr, "leg 1\n"));
        return ((_readByte<<shift) & 0xFF) >> (shift + (_toRead-=kBits));
    }
    else
    {
        HCDBG(fprintf(stderr, "leg 2 _readByte is %d, shift %d\n", _readByte, shift));
        int result = _toRead > 0 ? ((_readByte << shift) & 0xFF) >> shift : 0;
        HCDBG(fprintf(stderr, "result is %d\n", result));
        for (kBits -= _toRead; kBits >= BitsInByte; kBits -= BitsInByte)
        {
            int foo = getNextByte();
            HCDBG(fprintf(stderr, "byte is %d\n", foo));
            result = (result << BitsInByte) | foo;
            HCDBG(fprintf(stderr, "and result is %d\n", result));
        }
        if (kBits > 0)
        {
            int foo = getNextByte();
            HCDBG(fprintf(stderr, "and byte is %d\n", foo));
            int thing = BitsInByte - kBits;
            HCDBG(fprintf(stderr, "thing is %d\n", thing));
            _toRead = thing;
            _readByte = foo;
            int right = (_readByte >> _toRead);
            HCDBG(fprintf(stderr, "right is %d\n", right));
            int left = result << kBits;
            HCDBG(fprintf(stderr, "kbits are %d\n", kBits));
            HCDBG(fprintf(stderr, "left is %d\n", left));
            int ret = left | right;
//          int ret = (result << kBits) | ((_readByte = foo) >> (_toRead = BitsInByte - kBits));
            HCDBG(fprintf(stderr, "and final is %d\n", ret));
            return ret;
        }
        else
        {
            _toRead = 0;
            HCDBG(fprintf(stderr, "and this result says %d\n", result));
            return result;
        }
    }
}

bool Decompressor::readNext(int k, CompressorIterator &it)
{
    if (read() != 0)
    {
        it.value(_path | read(k));
        return true;
    }
    else
    {
        for (int count = 1;; _readByte = getNextByte(), _toRead = BitsInByte)
        {
            while (_toRead-- > 0)
            {
                if ((_readByte & (1 << _toRead)) != 0)
                {
                    int saved = _path;
                    _path = ((_path >> (k + count) << count) | read(count)) << k;
                    if (_path != saved)
                    {
                        it.value(_path | read(k));
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    ++count;
                }
            }
        }
    }
}

void Decompressor::decode(int k, IntegerArray &array)
{
    for (int path = 0;;)
    {
        if (read() != 0)
        {
            array.push_back(path | read(k));
        }
        else
        {
            int count = countZeroes() + 1;
            int saved = path;
            path = ((path >> (k + count) << count) | read(count)) << k;
            if (path != saved)  // convention for end
                array.push_back(path | read(k));
            else
                break;
        }
    }
}

void Decompressor::ascDecode(int k, IntegerArray &array)
{
    for (int path = 0, start = 0;;)
    {
        HCDBG(fprintf(stderr, "path is %d, start is %d\n", path, start));
        if (read() != 0)
        {
            int inread = read(k);
            start += path | inread;
            HCDBG(fprintf(stderr, "inread is %d\n", inread));
            int final = start;
            HCDBG(fprintf(stderr, "1:Decompressor::ascDecode to %d\n", final));
            array.push_back(final);
        }
        else
        {
            int count = countZeroes() + 1;
            HCDBG(fprintf(stderr, "count is %d\n", count));
            int saved = path;
            int inread = read(count);
            HCDBG(fprintf(stderr, "inread is %d, k is %d, path is %d\n", inread,
                k, path));
            path = ((path >> (k + count) << count) | inread) << k;
            if (path != saved)  // convention for end
            {
                int anotherread = read(k);
                HCDBG(fprintf(stderr, "newinread is %d\n", anotherread));
                start += path | anotherread;
                int final = start;
                HCDBG(fprintf(stderr, "2:Decompressor::ascDecode to %d\n", final));
                array.push_back(final);
            }
            else
            {
                break;
            }
        }
    }
}

int Decompressor::ascendingDecode(int k, int start, std::vector<int> &array)
{
    int path = 0, index = 0;
    while (true)
    {
        if (read() != 0)
            array[index++] = (start += path | read(k));
        else
        {
            outerbreak hack = donothing;
            for (int cnt = 0;; _readByte = getNextByte(), _toRead = BitsInByte)
            {
                while (_toRead-- > 0)
                {
                    if ((_readByte & (1 << _toRead)) != 0)
                    {
                        ++cnt;
                        int Path = ((path >> (k + cnt) << cnt) | read(cnt)) << k;
                        if (Path != path)
                        {
                            array[index++] = (start += (path = Path) | read(k));
                            hack = docontinue;
                            break;
                        }
                        else
                            return index;
                    }
                    else
                        ++cnt;
                }
                if (hack == docontinue)
                    break;
            }
        }
    }
}

class StreamDecompressor : public Decompressor
{
private:
    std::ifstream *_input;
public:
    StreamDecompressor(std::ifstream &input) { initReading(input); }
    using Decompressor::initReading;
    virtual void initReading(std::ifstream &input) { _input = &input; Decompressor::initReading(); }
    int getNextByte()
    {
        unsigned char ret;
        _input->read( (char*)&ret, 1 );
        HCDBG(fprintf(stderr, "StreamDecompressor::getNextByte of %d\n", ret));
        return ret;
    }
};

void Index::readPositions()
{
    getPositionsFile();
    //!!! temporary: better than fixed large value, worse than 'intelligent' size mgt
    _positionsFile->seekg(0, std::ios::end);
    _positionsCacheSize = _positionsFile->tellg();
    if (_positionsCacheSize < 0) _positionsCacheSize = 0;
    _positionsFile->clear();
    _positionsFile->seekg(0, std::ios::beg);

    if (_positionsCacheSize <= _positionsCacheSize)
    {
        _allInCache = true;
        _positions.resize(_positionsCacheSize);
        _positionsFile->readsome((char*)(&_positions[0]), _positionsCacheSize);
        std::cout << "POS fits in cache" << std::endl;
    }
}

void Index::readOffsetsTables(const std::string &fileName)
{
    std::ifstream in(indexFile(fileName).native_file_string().c_str(), std::ios::binary);
    unsigned char k1;
    in.read( (char*)&k1, 1 );
    StreamDecompressor sddocs(in);
    sddocs.decode(k1, _documents);
    unsigned char k2;
    in.read( (char*)&k2, 1 );
    _microIndexOffsets.clear();
    StreamDecompressor sdoffsets(in);
    sdoffsets.ascDecode(k2, _microIndexOffsets);
    // decompress titles' ids table
    unsigned char k3;
    in.read( (char*)&k3, 1 );
    _titles.clear();
    StreamDecompressor sdtitles(in);
    sdtitles.decode(k3, _titles);
}

void Index::readDocumentsTable(const std::string &fileName)
{
    std::ifstream in(indexFile(fileName).native_file_string().c_str(), std::ios::binary);
    unsigned char k1;
    in.read( (char*)&k1, 1 );
    _concepts.clear();
    StreamDecompressor sddocs(in);
    sddocs.ascDecode(k1, _concepts);
    unsigned char k2;
    in.read( (char*)&k2, 1 );
    _offsets.clear();
    _offsets.push_back(0);
    StreamDecompressor sdoffsets(in);
    sdoffsets.ascDecode(k2, _offsets);
    in.close();
}

class ContextTables;

class Tables
{
private:
    std::vector<int> _initialWordsCached;
    std::vector<int> _destsCached;
    std::vector<int> _linkTypesCached;
    std::vector<int> _seqNumbersCached;
public:
    Tables(const std::vector<int> &initialWords,
                  std::vector<int> &dests,
                  std::vector<int> &linkTypes,
                  std::vector<int> &seqNumbers)
    {
        _initialWordsCached = initialWords;
        _destsCached = dests;
        _linkTypesCached = linkTypes;
        _seqNumbersCached = seqNumbers;
    }
    void setTables(ContextTables &context);
}; // end of Tables

class ContextTables
{
public:
    std::vector<int> _initialWords;
    std::vector<int> _dests;
    std::vector<int> _linkTypes;
    std::vector<int> _seqNumbers;
    int _nTextNodes;
private:
    std::vector<Tables*> _cache;
    // cached last position for linear search
    int _initialWordsIndex;
    // link names are shared between all microindexes in an index
    std::vector<std::string> _linkNames;
    // offsets to tables' storage in file (or memory)
    std::vector<int> _offsets;
    std::vector<unsigned char> _contextData;  // !!! fully cached for now
    // auxillary
    IntegerArray _kTable;
    // _auxArray will be used as an auxillary to decode arrays
    IntegerArray _auxArray;
    int _lastDocNo;

    std::vector<int> _markers;

public:
    ContextTables(const std::vector<int> &offsets, const std::vector<unsigned char> &contextData,
        const std::vector<std::string> &linkNames);
    ~ContextTables();
    void setMicroindex(int docNo);
    int parentContext(int context);
    const std::string& linkName(int context);
    int linkCode(const std::string &linkName);
    std::vector<bool> getIgnoredElementsSet(const std::vector<std::string> &ignoredElements);
    bool notIgnored(int ctx, const std::vector<bool> &ignoredElements);
    int firstParentWithCode(int pos, int linkCode);
    int firstParentWithCode2(int pos, int linkCode, int parentCode);
    int firstParentWithCode3(int pos, int linkCode, int ancestorCode);
    int firstParentWithCode4(int pos, const std::vector<int> &linkCodes);
    int firstParentWithCode5(int pos, const std::vector<int> &pathCodes);
    int firstParentWithCode7(int pos, int linkCode, int seq);
    bool isGoverning(int context) { return linkName(context) == "TITLE"; }
    void resetContextSearch() { _initialWordsIndex = 0; }
private:
    void appendSegment(int context, std::string &result);
    int findIndexBin(int wordNumber);
public:
    int wordContextLin(int wordNumber);
};

ContextTables::ContextTables(const std::vector<int> &offsets, const std::vector<unsigned char> &contextData,
        const std::vector<std::string> &linkNames) : _kTable(5), _auxArray(4096), _lastDocNo(-1)
{
    _offsets = offsets;
    _contextData = contextData;
    _linkNames = linkNames;
    _cache.resize(_offsets.size());
}

ContextTables::~ContextTables()
{
    for (size_t i = 0; i < _cache.size(); ++i)
        delete _cache[i];
}

void ContextTables::setMicroindex(int docNo)
{
    if (docNo != _lastDocNo) // check if we need to do anything
    {
        if (_cache[docNo])
            _cache[docNo]->setTables(*this);
        else
        {
            int offset = _offsets[docNo];
            int k0 = _contextData[offset] & 0xFF;
            ByteArrayDecompressor compr(&_contextData, offset + 1);
            _kTable.clear();
            compr.decode(k0, _kTable);
            // decompress initialWords into auxiliary array
            _auxArray.clear();
            compr.ascDecode(_kTable[0], _auxArray); // _initialWords
            _initialWords = _auxArray;
            _nTextNodes = _initialWords.size();
            // decompress destinations into auxiliary array
            _auxArray.clear();
            compr.decode(_kTable[1], _auxArray); // _dests
            _auxArray.push_back(-1);      // sentinel, root
            _dests = _auxArray;
            _linkTypes.clear();
            compr.decode(_kTable[2], _linkTypes);
            _seqNumbers.clear();
            compr.decode(_kTable[3], _seqNumbers);

            _cache[docNo] = new Tables(_initialWords, _dests, _linkTypes, _seqNumbers);

            /*
            System.out.println("|_initialWords| = " + _nTextNodes);
            System.out.println("|_dests| -1 = " + (_dests.length - 1));
            System.out.println("|_seqNumbers| = " + _seqNumbers.length);
            System.out.println("|_linkTypes| = " + _linkTypes.length);
            */
        }
        _lastDocNo = docNo;
        _markers.resize(_dests.size());
    }
    _initialWordsIndex = 0;
}

int ContextTables::parentContext(int context)
{
    return _dests[context];
}

const std::string& ContextTables::linkName(int context)
{
    return _linkNames[_linkTypes[context]];
}

int ContextTables::linkCode(const std::string &inlinkName)
{
    for (size_t i = 0; i < _linkNames.size(); i++)
        if (inlinkName == _linkNames[i])
            return i;
    return -1;                  // when not found
}

std::vector<bool> ContextTables::getIgnoredElementsSet(const std::vector<std::string> &ignoredElements)
{
    std::vector<bool> result;
    bool noValidIgnoredElements = true;
    if (!ignoredElements.empty())
    {
        result.resize(_linkNames.size());
        for (size_t i = 0; i < ignoredElements.size(); i++)
        {
            int code = linkCode(ignoredElements[i]);
            if (code > -1)
            {
                result[code] = true;
                noValidIgnoredElements = false;
            }
        }
    }
    return noValidIgnoredElements ? std::vector<bool>() : result;
}

bool ContextTables::notIgnored(int ctx, const std::vector<bool> &ignoredElements)
{
    do
    {
        if (ignoredElements[_linkTypes[ctx]])
        {
            std::cout << "hit ignored" << std::endl;
            return false;
        }
    }
    while ((ctx = _dests[ctx]) > -1); // parentContext 'hand inlined'
    return true;
}

/** starting with ctx and going up the ancestry tree look for the first
  context with the given linkCode */
int ContextTables::firstParentWithCode(int pos, int inlinkCode)
{
    int ctx = _dests[wordContextLin(pos)]; // first parent of text node
    int shift = _nTextNodes;
    int limit = _dests.size() - 1;
    while (_linkTypes[ctx - shift] != inlinkCode)
        if ((ctx = _dests[ctx]) == limit)
            return -1;
    return ctx;
}

/** starting with ctx and going up the ancestry tree look for the first
  context with the given linkCode and given parent code */
int ContextTables::firstParentWithCode2(int pos, int inlinkCode, int parentCode)
{
    int ctx = _dests[wordContextLin(pos)]; // first parent of text node
    int shift = _nTextNodes;
    int limit = _dests.size() - 1;
    for (int parent = _dests[ctx]; parent < limit; parent = _dests[parent])
        if (_linkTypes[parent - shift] == parentCode && _linkTypes[ctx - shift] == inlinkCode)
            return ctx;
        else
            ctx = parent;
    return -1;
}

/** starting with ctx and going up the ancestry tree look for the first
  context with the given linkCode and given ancestor code */
int ContextTables::firstParentWithCode3(int pos, int inlinkCode, int ancestorCode)
{
    int ctx = _dests[wordContextLin(pos)];
    int shift = _nTextNodes;
    int limit = _dests.size() - 1;
    // find first instance of linkCode
    while (ctx < limit && _linkTypes[ctx - shift] != inlinkCode)
      ctx = _dests[ctx];
    if (ctx < limit)            // found linkCode, check ancestry
      for (int ancestor = _dests[ctx];
           ancestor < limit;
           ancestor = _dests[ancestor])
        if (_linkTypes[ancestor - shift] == ancestorCode) // ancestor confirmed
          return ctx;           // match found, return successful ctx
    return -1;                  // match NOT found
}

/** starting with ctx and going up the ancestry tree look for the first
  context with any of the given linkCode */
int ContextTables::firstParentWithCode4(int pos, const std::vector<int> &linkCodes)
{
    int nCodes = linkCodes.size();
    int shift = _nTextNodes;
    int limit = _dests.size() - 1;
    for (int ctx = _dests[wordContextLin(pos)]; ctx < limit; ctx = _dests[ctx])
    {
        int code = _linkTypes[ctx - shift];
        for (int i = 0; i < nCodes; i++)
            if (code == linkCodes[i])
                return ctx;
    }
    return -1;
}

/** starting with ctx and going up the ancestry tree look for the first
  context with the given path */
int ContextTables::firstParentWithCode5(int pos, const std::vector<int> &pathCodes)
{
    int nCodes = pathCodes.size();
    int lastCode = pathCodes[nCodes - 1];
    int shift = _nTextNodes;
    int limit = _dests.size() - 1;
    int ctx = _dests[wordContextLin(pos)];
    for (int parent = _dests[ctx]; parent < limit; parent = _dests[parent])
    {
        if (_linkTypes[ctx - shift] == lastCode)
        {
            // try to match the entire path
            outerbreak hack = donothing;
            for (int i = nCodes - 2, parent2 = parent; i >= 0; i--)
                if (_linkTypes[parent2 - shift] != pathCodes[i]) // match failure
                {
                    hack = docontinue;
                    break;    // try to match higher
                }
                else if ((parent2 = _dests[parent2]) == limit)
                    return -1;
            if (hack == docontinue)
                continue;
            return ctx;
        }
        else
            ctx = parent;
    }
    return -1;
}

/** starting with ctx and going up the ancestry tree look for the first
  context with the given linkCode */
int ContextTables::firstParentWithCode7(int pos, int inlinkCode, int seq)
{
    int ctx = _dests[wordContextLin(pos)]; // first parent of text node
    int shift = _nTextNodes;
    int limit = _dests.size() - 1;
    while (_linkTypes[ctx - shift] != inlinkCode || _seqNumbers[ctx] != seq)
      if ((ctx = _dests[ctx]) == limit)
        return -1;
    return ctx;
}

void ContextTables::appendSegment(int context, std::string &result)
{
    result.append(context < _nTextNodes ? "text()" : _linkNames[_linkTypes[context - _nTextNodes]]);
    result.push_back('[');
    std::ostringstream tmp;
    tmp << _seqNumbers[context];
    result.append(tmp.str());
    result.append("]/");
}

int ContextTables::findIndexBin(int wordNumber)
{
    int i = 0, j = _nTextNodes - 1;
    while (i <= j)
    {
      int k = (i + j) >> 1;
      if (_initialWords[k] < wordNumber)
        i = k + 1;
      else if (_initialWords[k] > wordNumber)
        j = k - 1;
      else
        return k;
    }
    return i - 1;
}

int ContextTables::wordContextLin(int wordNumber)
{
    for (int i = _initialWordsIndex; i < _nTextNodes; i++)
      if (_initialWords[i] > wordNumber) // first such i
      {
        // - 1 if wordNumbers can be the same
        _initialWordsIndex = i; // cached to speed up next search
        return i - 1;
      }
    return _nTextNodes - 1;
}

void Tables::setTables(ContextTables &context)
{
    context._initialWords = _initialWordsCached;
    context._dests = _destsCached;
    context._linkTypes = _linkTypesCached;
    context._seqNumbers = _seqNumbersCached;
    context._nTextNodes = context._initialWords.size();
}

class Compressor;

class XmlIndex : public Index
{
private:
    VectorBtreeParameters *_edgesParams;
    FullVectorBtree *_edges;
    ContextTables *_contextTables;
    std::fstream *_contextsFile;
    IntegerArray _contextsOffsets;
    std::vector<unsigned char> _contextsData;
    std::vector<std::string> _linkNames;
protected:
    virtual void writeOutOffsets();
public:
    XmlIndex(const fs::path &index, bool update)
        : Index(index, update), _edgesParams(0), _edges(0), _contextTables(0), _contextsFile(0) {}
    void init();
    void close();
    virtual ~XmlIndex() { delete _edgesParams; delete _edges; delete _contextTables; }
    std::fstream& getContextsFile();
    using Index::compress;
    virtual void compress(int docID, int titleID,
        std::vector<ConceptLocation> &locations,
        std::vector<ConceptLocation> &extents,
        int k, const Compressor &contextTables);
    const std::vector<std::string>& getLinkNames() { return _linkNames; }
};

void XmlIndex::init()
{
    Index::init();
    if (_edgesParams) delete _edgesParams;
    _edgesParams = new VectorBtreeParameters(*_schema, "EDGE", 9);
    if (_edgesParams->readState() == false)
        _edgesParams->setBlockSize(1024);
    _edges = new FullVectorBtree(_edgesParams, _update);
    if (!_contextsOffsets.empty())
    {
        _contextsData = readByteArray("CONTEXTS");
#if 0
        _linkNames = (String[])readObject("LINKNAMES");
#endif
        _contextTables = new ContextTables(_contextsOffsets, _contextsData, _linkNames);
    }
}

void XmlIndex::writeOutOffsets()
{
    Index::writeOutOffsets();
    if (!_contextsOffsets.empty())
    {
        std::fstream &out = getOffsetsFile();
        Compressor offsets2;
        char k = static_cast<char>(offsets2.compressAscending(_contextsOffsets));
        out.write( (const char*)&k, 1 );
        offsets2.write(out);
    }
}

std::fstream& XmlIndex::getContextsFile()
{
    if (!_contextsFile)
        _contextsFile = getRAF("CONTEXTS", _update);
    return *_contextsFile;
}

void XmlIndex::close()
{
    if (_contextsFile)
    {
        _contextsFile->close();
        delete _contextsFile;
       _contextsFile = 0;
    }
    _edges->close();
    if (_update)
        _edgesParams->updateSchema();
    Index::close();
}

class Tokenizer
{
private:
    UnicodeString s;
    BreakIterator *bi;
    int32_t start;
    UConverter *utf8;
    std::vector<char> utfbuffer;
public:
    Tokenizer();
    ~Tokenizer();
    void setText(const xmlChar *text);
    std::string nextToken();
};

Tokenizer::Tokenizer() : start(BreakIterator::DONE), utfbuffer(64)
{
    UErrorCode status = U_ZERO_ERROR;
    bi = BreakIterator::createWordInstance("en_US", status);
    utf8 = ucnv_open("utf-8", &status);
}

Tokenizer::~Tokenizer()
{
#if !defined(SOLARIS)
    delete bi;
    ucnv_close(utf8);
#endif
}

void Tokenizer::setText(const xmlChar *text)
{
    UErrorCode status = U_ZERO_ERROR;
    s = UnicodeString((const char*)text, -1, utf8, status);
    bi->setText(s);
    start = ubrk_first(bi);
}

std::string Tokenizer::nextToken()
{
    std::string ret;

    int32_t end = ubrk_next(bi);
    while (end != BreakIterator::DONE)
    {
        if (ubrk_getRuleStatus(bi) != UBRK_WORD_NONE)
            break;
        start = end;
        end = ubrk_next(bi);
    }

    if (end != -1 && end != start)
    {
        UnicodeString token(s, start, end-start);
        token = token.toLower();
        size_t needed = 0;

        UErrorCode status = U_ZERO_ERROR;
        while ((needed = token.extract(&utfbuffer[0], utfbuffer.size(), utf8, status)) > utfbuffer.size())
            utfbuffer.resize(utfbuffer.size() * 2);

        ret = std::string(&utfbuffer[0], needed);
        start = end;
    }

    return ret;
}

typedef std::vector<xmlNodePtr> Vector;

ConceptLocation::ConceptLocation(int conceptID, int begin, int end) :
    _concept(conceptID), _begin(begin), _end(end)
{
}

#ifdef EMULATEORIGINALSORT
class ConceptLocationSorter
{
public:
    virtual bool smallerThan(const ConceptLocation &a, const ConceptLocation &b) = 0;
private:
    // part of quicksearch
    int partition(std::vector<ConceptLocation> &array, int p, int r)
    {
        ConceptLocation x = array[(p + r)/2];
        int i = p - 1, j = r + 1;
        while (true)
        {
            while (smallerThan(x, array[--j]))
                ;
            while (smallerThan(array[++i], x))
                ;
            if (i < j)
            {
                ConceptLocation t = array[i];
                array[i] = array[j];
                array[j] = t;
            }
            else
                return j;
        }
    }
public:
    void quicksort(std::vector<ConceptLocation> &array, int p, int r)
    {
        while (p < r)
        {
            int q = partition(array, p, r);
            quicksort(array, p, q);
            p = q + 1;
        }
    }
};

class ConceptSorter : public ConceptLocationSorter
{
public:
    bool smallerThan(const ConceptLocation &a, const ConceptLocation &b)
    {
        return a._concept < b._concept;
    }
};

class PositionSorter : public ConceptLocationSorter
{
public:
    bool smallerThan(const ConceptLocation &a, const ConceptLocation &b)
    {
        return a._begin < b._begin || a._begin == b._begin && a._end < b._end;
    }
};

#else

class ConceptSorter
{
public:
    bool operator()(const ConceptLocation &a, const ConceptLocation &b) const
    {
        return a._concept < b._concept;
    }
};

class PositionSorter
{
public:
    bool operator()(const ConceptLocation &a, const ConceptLocation &b) const
    {
        return a._begin < b._begin || (a._begin == b._begin && a._end < b._end);
    }
};

#endif

void ConceptLocation::sortByPosition(std::vector<ConceptLocation> &array, int i1, int i2)
{
#ifdef EMULATEORIGINALSORT
    PositionSorter _pComp;
    _pComp.quicksort(array, i1, i2 - 1);
#else
    std::vector<ConceptLocation>::iterator begin = array.begin();
    std::vector<ConceptLocation>::iterator end = begin;
    std::advance(begin, i1);
    std::advance(end, i2);
    std::sort(begin, end, PositionSorter());
#endif
}

void ConceptLocation::sortByConcept(std::vector<ConceptLocation> &array, int i1, int i2)
{
#ifdef EMULATEORIGINALSORT
    ConceptSorter _cComp;
    _cComp.quicksort(array, i1, i2 - 1);
#else
    std::vector<ConceptLocation>::iterator begin = array.begin();
    std::vector<ConceptLocation>::iterator end = begin;
    std::advance(begin, i1);
    std::advance(end, i2);
    std::sort(begin, end, ConceptSorter());
#endif
}

typedef std::map<xmlNodePtr, int> NodeHashtable;
typedef std::hash_map<std::string, int, pref_hash> LinkHashTable;

class IndexAdapter
{
private:
    static int StackSize;
    const char* _indexText_Name;
    const char* _indexElement_Name;
    const char* _indexAttribute_Name;
    const char* _nodeID_Name;
    const char* _tokenizer_Name;
    const char* _attributeName_Name;
    std::vector<bool> _indexOnOffStack;
    int _sp;
    int _tsp;
    std::vector< std::string > _attributeStack;
    xmlNodePtr _currentNode;
    int _attrSP;
    void storeLocation(const std::string &token, int number);
    void storeLocation(const std::string &token) { storeLocation(token, _lastWordNumber++); }
    void storeEdge(int relation, int seqNumber, int destination);

    void startElement(xmlNodePtr node);
    void attribute(const char *name, const char *value);
    void characters(const xmlChar *str) throw( HelpProcessingException );
    void endElement(xmlNodePtr node);

    void indexText(const xmlChar *str);

    Vector _textNodes;
    NodeHashtable _numberedNodes;
public:
    HashSet _stoplist;
    LinkHashTable _linkCodes;
    std::vector<std::string> _linknames;
    static int CurrenMaxLinkCode;
    std::vector<ConceptLocation> _locations;
    int _availContextNumber;
    IntegerArray _initialWords;
    IntegerArray _links;
    IntegerArray _dests;
    IntegerArray _seqNumbers;
    int _lastWordNumber;
    int _firstWord;
    bool _anyLocationsStored;
    XmlIndex *_index;
private:
    static int InitSize;
    int _size;
public:
    IndexAdapter();
    void process(xmlNodePtr node, xmlDocPtr doc);
    void init();
    void finish();
    int intern(const std::string &name) { return _index->intern(name); }
    int getLinkCode(const std::string &linkName);
};

int IndexAdapter::StackSize = 64;
int IndexAdapter::InitSize = 4096;
int IndexAdapter::CurrenMaxLinkCode = 0;

IndexAdapter::IndexAdapter()
    : _indexOnOffStack(StackSize), _attributeStack(StackSize),
    _anyLocationsStored(false), _size(InitSize)
{
    _indexText_Name = "text";
    _indexElement_Name = "element";
    _indexAttribute_Name = "attribute";
    _nodeID_Name = "nodeID";
    _tokenizer_Name = "tokenizer";
    _attributeName_Name = "attributeName";
}

void IndexAdapter::storeLocation(const std::string &token, int number)
{
    int concept = intern(token);
    HCDBG(std::cerr << "storeLocation of number " << number << "for token "
        << token << " as conceptlocation " << concept << std::endl);
    _locations.push_back(ConceptLocation(concept, number, number));
}

void IndexAdapter::storeEdge(int relation, int seqNumber, int destination)
{
    _links.push_back(relation);
    _seqNumbers.push_back(seqNumber);
    _dests.push_back(destination);
    HCDBG(std::cerr << "storeEdge" << std::endl);
}

void IndexAdapter::finish()
{
    _numberedNodes.clear();
    _dests.clear();
    _seqNumbers.clear();
    _links.clear();

    int nTextNodes = _textNodes.size();
    _availContextNumber = nTextNodes;
    // vector to hold parents of text nodes
    Vector parents;
    /*****
      for each of the text nodes its sequence number is stored
      as well as the index of its parent (in _dests)
      _link is not stored as it is always "text()"
      _availContextNumber only used to number parent element contexts
    ******/
    for (int i = 0; i < nTextNodes; i++)
    {
        xmlNodePtr node = _textNodes[i];
        xmlNodePtr parent = node->parent;
        // find this text node's seq number
        int counter = 1;
        xmlNodePtr sibling = parent->xmlChildrenNode;
        while (sibling && sibling != node)
        {
            if (xmlNodeIsText(sibling))
                ++counter;
            sibling = sibling->next;
        }
        _seqNumbers.push_back(counter);
        // check whether parent already encountered
        NodeHashtable::const_iterator number = _numberedNodes.find(parent);
        if (number == _numberedNodes.end()) // not yet seen
        {
            int newContext = _availContextNumber++;
            _numberedNodes.insert(NodeHashtable::value_type(parent, newContext)).first->second = newContext;
            _dests.push_back(newContext);
            // enqueue parent: its parent will need a number too
            parents.push_back(parent);
            //  System.out.println(parent.getName().toString() +
            //       " -> " + newContext);
        }
        else
        {
            _dests.push_back(number->second);
        }
    } // end for

    _textNodes.clear();

    // store info about element ancestry of the above text nodes
    // grandparents are added to the end of the vector
    int rootElementPos = 0;
    for (size_t i = 0; i < parents.size(); i++)
    {
        xmlNodePtr node = parents[i];

        std::string name((const char*)(node->name));

        xmlNodePtr parent = node->parent;

        _links.push_back(getLinkCode(name));

//        if (parent.getType() == Node.ELEMENT) // not ROOT
        if (parent && parent->parent) // not ROOT
        {
            // find sequence number
            xmlNodePtr sibling = parent->xmlChildrenNode;
            int counter = 1;
            while (sibling && sibling != node)
            {
                if (strcmp((const char*)sibling->name, (const char*)name.c_str()) == 0)
                    ++counter;
                sibling = sibling->next;
            }

            _seqNumbers.push_back(counter);

            // check whether parent already known
            NodeHashtable::iterator number = _numberedNodes.find(parent);
            if (number == _numberedNodes.end())
            {
                int newContext = _availContextNumber++;
                _numberedNodes.insert(NodeHashtable::value_type(parent, newContext)).first->second = newContext;
                _dests.push_back(newContext);
                // enqueue parent: its parent will need a number too
                parents.push_back(parent);
                //System.out.println(parent.getName().toString() +
                //   " -> " + newContext);
            }
            else
            {
                _dests.push_back(number->second);
            }
        }
        else
        {
            _dests.push_back(0); // placeholder
            _seqNumbers.push_back(1);
            rootElementPos = i + nTextNodes;
            //        System.out.println("rootElementPos = " + i);
        }
    } // end for

    if (_dests.empty())
        _dests.push_back(0);

    // index to sentinel
    _dests[rootElementPos] = _availContextNumber;
} // end public void finish

void IndexAdapter::init()
{
    _sp = -1;
    _tsp = -1;
    _attrSP = -1;
    _lastWordNumber = 0;
    _anyLocationsStored = false;
    _availContextNumber = 0;
    // all the contexts' tables
    _initialWords.clear();
    _locations.clear();
}

void IndexAdapter::attribute(const char *name, const char *value)
{
    HCDBG(std::cerr << "attribute: " << name << " = " << value << std::endl);
    if (strcmp(name, _nodeID_Name) == 0)
        _currentNode = (xmlNodePtr)(strtol(value, NULL, 10));
    else if (strcmp(name, _tokenizer_Name) == 0)
    {
        if (strcmp(value, "com.sun.xmlsearch.util.SimpleTokenizer") != 0 && !isExtensionMode() )
            std::cerr << "changing tokenizers not implemented in C++ version of HelpLinker"
            << " because no other tokenizers were referenced in the helpcontent2 source"
            << std::endl;
    }
    else if (strcmp(name, _attributeName_Name) == 0)
    {
        //namespace prefix ?
        std::string attrVal = std::string("index:") + value;
        if( !isExtensionMode() )
            std::cout << "attrVal = " << attrVal << std::endl;
        _attributeStack[_attrSP] = std::string(name) + '<' + value + '<' + attrVal;
        storeLocation("+<" + _attributeStack[_attrSP]);
    }
}

void IndexAdapter::indexText(const xmlChar *text)
{
    static Tokenizer tokenizer;
    tokenizer.setText(text);
    _firstWord = _lastWordNumber;
    _anyLocationsStored = false;

    std::string lowercaseToken = tokenizer.nextToken();
    while (!lowercaseToken.empty())
    {
        HCDBG(std::cerr << "token is: " << lowercaseToken << std::endl);
#ifdef EMULATEORIGINAL
        if ((lowercaseToken.size() == 1) && isdigit(lowercaseToken[0]))
        {
            lowercaseToken = tokenizer.nextToken();
            continue;
        }
#endif
        if (std::find(_stoplist.begin(),
          _stoplist.end(), lowercaseToken) == _stoplist.end())
        {
            storeLocation(lowercaseToken);
            _anyLocationsStored = true;
        }
        else
            _lastWordNumber++;
        lowercaseToken = tokenizer.nextToken();
    }

    if (_anyLocationsStored && _firstWord > -1)
    {
        _initialWords.push_back(_firstWord);
        HCDBG(std::cerr << "appending " << _firstWord << std::endl);
        _textNodes.push_back(_currentNode);
    }
    // reset before next batch
    _firstWord = -1;
}

void IndexAdapter::characters(const xmlChar *str) throw( HelpProcessingException )
{
    if (!str)
    {
        std::stringstream aStrStream;
        aStrStream << "no characters!" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_INTERNAL_ERROR, aStrStream.str() );
    }

    HCDBG(std::cerr << "IndexAdapter::characters of " << str << std::endl);
    HCDBG(std::cerr << _sp << " : " <<  _indexOnOffStack[_sp] << std::endl);

    if (_sp >= 0 && _indexOnOffStack[_sp])
    {
        indexText( str );
    }
}

void IndexAdapter::startElement(xmlNodePtr node)
{
    const char *name = (const char*)(node->name);

    HCDBG(std::cerr << "startElement is " << name << std::endl);

    if (strcmp(name, _indexElement_Name) == 0)
    {
        _indexOnOffStack[++_sp] = true;
        // pop Tokenizer stack
        // following attribute can push selected Tokenizer
        if (_tsp != -1)
            _tsp--;
    }
    else if (strcmp(name, _indexText_Name) == 0)
    {
    }
    else if (strcmp(name, _indexAttribute_Name) == 0)
    {
        _attrSP++;
    }
}

void IndexAdapter::endElement(xmlNodePtr node)
{
    const char *name = (const char*)(node->name);
    HCDBG(std::cerr << "endElement is " << name << std::endl);
    if (strcmp(name, _indexElement_Name) == 0)
        _sp--;
    else if (strcmp(name, _indexText_Name) == 0)
    {
        // reset
    }
    else if (strcmp(name, _indexAttribute_Name) == 0)
        storeLocation("-<" + _attributeStack[_attrSP--]);
}

int IndexAdapter::getLinkCode(const std::string &linkName)
{
    LinkHashTable::iterator code = _linkCodes.find(linkName);
    if (code != _linkCodes.end())
        return code->second;
    else
    {
        _linknames.push_back(linkName);
        int newCode = CurrenMaxLinkCode++;
        _linkCodes.insert(LinkHashTable::value_type(linkName, newCode)).first->second = newCode;
        return newCode;
    }
}

void IndexAdapter::process(xmlNodePtr node, xmlDocPtr doc)
{
    startElement(node);

    for (xmlAttrPtr attr = node->properties; attr; attr = attr->next)
    {
        xmlChar *value = xmlNodeListGetString(doc, attr->children, 0);
        attribute((const char*)(attr->name),  (const char*)value);
        xmlFree(value);
    }

    if (xmlNodeIsText(node))
    {
        xmlChar *str = xmlNodeListGetString(doc, node, 1);
        characters(str);
        xmlFree(str);
    }

    for (xmlNodePtr test = node->xmlChildrenNode; test; test = test->next)
        process(test, doc);

    endElement(node);
}

class XmlIndexBuilder
{
private:
    fs::path _transformLocation;
    xsltStylesheetPtr _indexingTransform;
    IndexAdapter _indexAdapter;
    int _currentDocID;
    void reset();
    xsltStylesheetPtr getTransform(const std::string &stylesheetName);
public:
    XmlIndexBuilder() : _indexingTransform(0) {}
    XmlIndexBuilder(const fs::path &dir);
    ~XmlIndexBuilder();
    void clearIndex();
    void setTransformLocation(const fs::path &filelocation);
    void init(const std::string &transform);
    void initXmlProcessor(const std::string &transform);
    void indexDocument(xmlDocPtr document, const std::string &docURL, const std::string &title);
    int intern(const std::string &name);
    void openDocument(const std::string &name) throw( HelpProcessingException );
    void closeDocument(const std::string &name) throw( HelpProcessingException );
    void close();
};

void XmlIndexBuilder::close()
{
    fs::path fullname = _indexAdapter._index->indexFile("LINKNAMES");
    std::fstream _linkFile(fullname.native_file_string().c_str(), std::ios::out | std::ios::trunc | std::ios::binary);

#ifdef EMULATEORIGINAL
    static const unsigned char vectorheader[] =
    {
        0xAC, 0xED, 0x00, 0x05, 0x75, 0x72, 0x00, 0x13,
        0x5B, 0x4C, 0x6A, 0x61, 0x76, 0x61, 0x2E, 0x6C,
        0x61, 0x6E, 0x67, 0x2E, 0x53, 0x74, 0x72, 0x69,
        0x6E, 0x67, 0x3B, 0xAD, 0xD2, 0x56, 0xE7, 0xE9,
        0x1D, 0x7B, 0x47, 0x02, 0x00, 0x00, 0x78, 0x70
    };

    _linkFile.write((const char*)(&vectorheader[0]), sizeof(vectorheader));
    writeInt(_linkFile, _indexAdapter._linknames.size());
    std::vector<std::string>::iterator aEnd = _indexAdapter._linknames.end();
    for (std::vector<std::string>::iterator aIter = _indexAdapter._linknames.begin();
        aIter != aEnd; ++aIter)
    {
        HCDBG(std::cerr << "linkname is " << *aIter << std::endl);
        _linkFile << 't';
        writeShort(_linkFile, aIter->size());
        _linkFile << *aIter;
    }
#else
    std::vector<std::string>::iterator aEnd = _indexAdapter._linknames.end();
    for (std::vector<std::string>::iterator aIter = _indexAdapter._linknames.begin();
        aIter != aEnd; ++aIter)
    {
        _linkFile << *aIter << '\n';
    }
#endif
#if 0

    // output link codes
    /*
        Enumeration keys = _linknames.elements();
        while (keys.hasMoreElements())
        System.out.println((String)keys.nextElement());
    */
#endif
    _indexAdapter._index->close();
    std::cout << "done" << std::endl;
}

int XmlIndexBuilder::intern(const std::string &name)
{
    return _indexAdapter.intern(name);
}

void XmlIndexBuilder::openDocument(const std::string &name) throw( HelpProcessingException )
{
    if (_currentDocID != 0)
    {
        std::stringstream aStrStream;
        aStrStream << "document already open" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_INTERNAL_ERROR, aStrStream.str() );
    }
    _currentDocID = intern( PrefixTranslator::translatePrefix(name) );
    reset(); // reset context gathering state
}

int BitBuffer::InitSize   = 256;
int BitBuffer::NBits      =  32;
int BitBuffer::BitsInByte =   8;
int BitBuffer::BytesInInt =   4;

void Compressor::encode(const IntegerArray &pos, int k)
{
    HCDBG(std::cerr << "1:start this encode of " << k << "size of "
        << pos.size() << std::endl);
    unsigned int n1 = 0;
    unsigned int power = 1 << k;
    for (size_t i = 0; i < pos.size(); i++)
    {
        HCDBG(std::cerr << "1: loop " << i << std::endl);
        unsigned int n2 = pos[i] >> k;
        int rem = pos[i] % power;
        HCDBG(std::cerr << "1: n1, n2 : " << n1 << "," << n2 << std::endl);
        if (n2 != n1)
        {
            unsigned int min = n1;
            unsigned int a = n1;
            int lev = 0, power2 = 1;
            if (n2 > n1)
                for (size_t max = n1; max < n2; a >>= 1, power2 <<= 1, lev++)
                    if ((a & 1) != 0)
                        min -= power2;
                    else
                        max += power2;
            else
                for ( ; min > n2; a >>= 1, power2 <<= 1, lev++)
                    if ((a & 1) != 0)
                        min -= power2;
            // lev 0s, 1, lev bits of (n2 - min) plus following value
            // no 'V' symbol needed here
            if (lev*2 + 1 + k <= NBits)
                _buffer.append((1<<lev | (n2 - min)) << k | rem, lev*2+1+k);
            else
            {
                if (lev*2 + 1 <= NBits)
                    _buffer.append(1 << lev | (n2 - min), lev*2 + 1);
                else
                {
                    _buffer.append(0, lev);
                    _buffer.append(1 << lev | (n2 - min), lev + 1);
                }
                _buffer.append(rem, k);
            }
            n1 = n2;
        }
        else
            _buffer.append(rem | power, k + 1); // 'V' + value
    }
    _buffer.append(2 | n1 & 1, 3); // marking end
    _buffer.close();
    HCDBG(std::cerr << "1:end this encode of " << k << std::endl);
}

void Compressor::encode(const IntegerArray &pos, const IntegerArray &len, int k, int k2)
{
    HCDBG(std::cerr << "2:start this encode of " << k << "size of "
        << pos.size() << std::endl);
    int power = 1 << k, n1 = 0;
    for (size_t i = 0; i < pos.size(); i++)
    {
        HCDBG(std::cerr << "2: loop " << i << std::endl);
        int n2 = pos[i] >> k;
        int rem = pos[i] % power;
        HCDBG(std::cerr << "2: n1, n2 : " << n1 << "," << n2 << std::endl);
        if (n2 != n1)
        {
            int min = n1, a = n1;
            int lev = 0, power2 = 1;
            if (n2 > n1)
                for (int max = n1; max < n2; a >>= 1, power2 <<= 1, lev++)
                    if ((a & 1) != 0)
                        min -= power2;
                    else
                        max += power2;
            else
                for ( ; min > n2; a >>= 1, power2 <<= 1, lev++)
                    if ((a & 1) != 0)
                        min -= power2;
            // lev 0s, 1, lev bits of (n2 - min) plus following value
            if (lev*2 + 1 + k <= NBits)
                _buffer.append((1<<lev | (n2 - min)) << k | rem, lev*2+1+k);
            else
            {
                if (lev*2 + 1 <= NBits)
                    _buffer.append(1 << lev | (n2 - min), lev*2 + 1);
                else
                {
                    _buffer.append(0, lev);
                    _buffer.append(1 << lev | (n2 - min), lev + 1);
                }
                _buffer.append(rem, k);
            }
            _buffer.append(len[i], k2);
            n1 = n2;
        }
        else
            _buffer.append((rem|power)<<k2 | len[i], k+k2+1); // 'V' + v1,v2
    }
    _buffer.append(2 | n1 & 1, 3); // marking end
    _buffer.close();
    HCDBG(std::cerr << "2:end this encode of " << k << std::endl);
}

// k: starting value for minimization
int Compressor::minimize(const IntegerArray &array, int startK)
{
    BitBuffer saved;
    int minK = startK;
    _buffer.clear();
    encode(array, startK);
    int min = _buffer.bitCount();   // init w/ first value
    saved.setFrom(_buffer);

    _buffer.clear();
    encode(array, startK + 1);

    if (_buffer.bitCount() < min)
    {
        int k = startK + 1;
        do
        {
            saved.setFrom(_buffer);
            min = _buffer.bitCount();
            minK = k;
            _buffer.clear();
            encode(array, ++k);
        }
        while (_buffer.bitCount() < min);
    }
    else                // try smaller values through 1
    {
        for (int k = startK - 1; k > 0; k--)
        {
            _buffer.clear();
            encode(array, k);
            if (_buffer.bitCount() < min)
            {
                saved.setFrom(_buffer);
                min = _buffer.bitCount();
                minK = k;
            }
            else
                break;
        }
    }

    _buffer.setFrom(saved);
    return minK;
}

int Compressor::compressAscending(const IntegerArray &array)
{
    IntegerArray differences(array.size());
    toDifferences(array, differences);
    return minimize(differences, BeginK);
}

int Compressor::NBits  = 32;
int Compressor::BeginK =  5;

class DocumentCompressor
{
public:
    static int NConceptsInGroup;
    static int BitsInLabel;
    static int DefaultSize;
private:
    int _nGroups;
    int _nExtents;
    unsigned int _freeComp;
    int _kk;
    Compressor  *_currentCompressor;
    std::vector<Compressor> _compressors;
    Compressor   _kCompr;
    Compressor   _lCompr;
    Compressor   _mCompr;
    Compressor   _posCompressor;
    IntegerArray _kTable; // k's for the series
    IntegerArray _lTable; // lengths of the C/P groups
    IntegerArray _maxConcepts; // maximal concepts in CP
    IntegerArray _concepts;
    IntegerArray _documents;
    IntegerArray _microIndexOffsets;
    IntegerArray _titles;
    // _contextsOffsets for use in XML indexing
    IntegerArray _contextsOffsets;
    IntegerArray _positions;
    IntegerArray _labels;

public:
    DocumentCompressor() : _currentCompressor(0), _compressors(DefaultSize) {}
    void writeOutMicroIndex(std::fstream &output,
                 std::vector<ConceptLocation> &locations,
                 std::vector<ConceptLocation> &extents)
    {
        HCDBG(std::cerr << "writeOutMicroIndex start" << std::endl);
        encode(locations, NConceptsInGroup);
        HCDBG(std::cerr << "writeOutMicroIndex end encode" << std::endl);
        if (!extents.empty())
            encodeExtents(extents);
        HCDBG(std::cerr << "writeOutMicroIndex finalize" << std::endl);
        finalizeEncoding();
        HCDBG(std::cerr << "writeOutMicroIndex write" << std::endl);
        writeOut(output);
        HCDBG(std::cerr << "writeOutMicroIndex end" << std::endl);
    }
private:
    void encode(std::vector<ConceptLocation> &locations, int nConcepts)
    {
        int initK = 4;
        // first sort by concept only
#ifdef CMCDEBUG
        for (size_t i = 0; i < locations.size(); ++i)
            fprintf(stderr, "unsorted is %d\n", locations[i].getConcept());
#endif
        HCDBG(std::cerr << "start sort" << std::endl);
        ConceptLocation::sortByConcept(locations, 0, locations.size());
        HCDBG(std::cerr << "end sort" << std::endl);
#ifdef CMCDEBUG
        for (size_t i = 0; i < locations.size(); ++i)
            fprintf(stderr, "sorted is %d\n", locations[i].getConcept());
#endif

        // using the fact that concepts are already sorted
        // count of groups of 'nConcepts'
        // go for differences directly

        // clear the state
        _nGroups = 0;
        _nExtents = 0;
        _kTable.clear();
        _lTable.clear();
        _concepts.clear();
        _maxConcepts.clear();
        _kCompr.clear();
        _lCompr.clear();
        _mCompr.clear();
        for (size_t i = 0; i < _compressors.size(); i++)
            _compressors[i].clear();
        _freeComp = 0;
        _currentCompressor = NULL;
        // end of resetting state

        int conceptCounter = 0;
        int fromIndex = 0;
        int prevMax = 0;
        int last = locations[0].getConcept(); // init w/ first ID
        nextCompressor();
        _concepts.push_back(last);
        for (size_t i = 0;;)
        {
            for (; i < locations.size() && locations[i].getConcept() == last; i++)
                locations[i].setConcept(conceptCounter);
            if (i == locations.size())
            {
                if (!_concepts.empty())
                {
                    ++_nGroups;
                    _kTable.push_back(_currentCompressor->minimize(_concepts, initK));
                }
                encodePositions(locations, fromIndex, i, BitsInLabel);
                break;
            }
            else
            {               // new concept (group?)
                if (++conceptCounter == nConcepts)
                {
                    ++_nGroups;
                    // we are looking at the beginning of a new group
                    // last is maximal for the group just finished
                    // it won't be stored in concepts array but maxConcepts
                    _concepts.pop_back();
                    HCDBG(fprintf(stderr, "_maxConcepts %d %d -> %d\n", last, prevMax, last - prevMax));
                    _maxConcepts.push_back(last - prevMax);
                    prevMax = last;
                    _kTable.push_back(_currentCompressor->minimize(_concepts, initK));

#ifdef CMCDEBUG
                    for(size_t p = 0; p < locations.size(); ++p)
                        std::cerr << "microindex2 this testing is " << locations[p].getBegin() <<
                            locations[p].getEnd() << " : " << locations[p].getConcept() << std::endl;
#endif

                    HCDBG(std::cerr << "two encodePositions " << fromIndex << " " << i << std::endl);
                    encodePositions(locations, fromIndex, i, BitsInLabel);
                    fromIndex = i;
                    nextCompressor();
                    _concepts.clear();
                    conceptCounter = 0;
                }
                _concepts.push_back(locations[i].getConcept() - last);
                last = locations[i].getConcept();
            }
        }
    }

    void encodePositions(std::vector<ConceptLocation> &locations, int from, int to, int cK)
    {
        int initK = 3;
        int lastPos, k;
        // sort in place by psitions only
#ifdef CMCDEBUG
        for (int i = from; i < to; ++i)
            fprintf(stderr, "unsorted is %d %d\n", locations[i].getBegin(), locations[i].getEnd());
#endif
        ConceptLocation::sortByPosition(locations, from, to);
#ifdef CMCDEBUG
        for (int i = from; i < to; ++i)
            fprintf(stderr, "sorted is %d %d\n", locations[i].getBegin(), locations[i].getEnd());
#endif
        _positions.clear();
        _labels.clear();
        _positions.push_back(lastPos = locations[from].getBegin());
        _labels.push_back(locations[from].getConcept()); // now: a label
        // skip duplicates
        for (int i = from, j = from + 1; j < to; j++)
        {
            if (locations[i].equals(locations[j]) == false)
            {
                i = j;
                HCDBG(std::cerr << "i is " << i << "locations begin is "
                    << locations[i].getBegin() << "last pos is " << lastPos << std::endl);
                _positions.push_back(locations[i].getBegin() - lastPos);
                lastPos = locations[i].getBegin();
                _labels.push_back(locations[i].getConcept()); // now: a label
            }
        }
        // first find k by minimizing just positions w/o labels
        _kTable.push_back(k = _posCompressor.minimize(_positions, initK));
        _posCompressor.clear();
        HCDBG(std::cerr << "start encodePositions" << std::endl);
        _posCompressor.encode(_positions, _labels, k, cK);
        HCDBG(std::cerr << "end encodePositions" << std::endl);
        _currentCompressor->concatenate(_posCompressor);
    }

    void encodeExtents(std::vector<ConceptLocation> &extents)
    {
        // side effects:
        // 'k3' added to _kTable
        // a number of compressors populated: header + lengths' lists
        int initK = 4;
        int c = 0;
        IntegerArray concepts; //difference
        IntegerArray lengths;
        IntegerArray kTable;
        IntegerArray lTable;
        // reserve a compressor for concatenated tables
        nextCompressor();
        Compressor *extentsHeader = _currentCompressor;
        std::vector<ConceptLocation>::const_iterator aEnd = extents.end();
        for (std::vector<ConceptLocation>::const_iterator aIter = extents.begin();
            aIter != aEnd; ++aIter)
        {
            if (aIter->getConcept() != c)
            {
                if (c != 0)
                {
                    _nExtents++;
                    nextCompressor();
                    kTable.push_back(_currentCompressor->minimize(lengths, initK));
                    lTable.push_back(_currentCompressor->byteCount());
                }
                concepts.push_back(aIter->getConcept() - c);
                c = aIter->getConcept();
                lengths.clear();
                lengths.push_back(aIter->getLength());
            }
            else
                lengths.push_back(aIter->getLength());
        }
        // last table of lengths
        nextCompressor();
        kTable.push_back(_currentCompressor->minimize(lengths, initK));
        lTable.push_back(_currentCompressor->byteCount());
        Compressor compressor1;
        kTable.push_back(compressor1.minimize(lTable, initK));
        Compressor compressor2;
        kTable.push_back(compressor2.minimize(concepts, initK));
        _kTable.push_back(extentsHeader->minimize(kTable, initK)); // k3
        extentsHeader->concatenate(compressor1);
        extentsHeader->concatenate(compressor2);
    }

    void finalizeEncoding()
    {
        if (_nGroups > 1)
        {
            // if extents follow C/P groups we need the length of the last group
            int limit = _nExtents > 0 ? _freeComp : _freeComp - 1;
            for (int j = 0; j < limit; j++) // length of last not saved
                _lTable.push_back(_compressors[j].byteCount());

            _kTable.push_back(_mCompr.minimize(_maxConcepts, 3));
            _kTable.push_back(_lCompr.minimize(_lTable, 3));
            _kk = _kCompr.minimize(_kTable, 3);
            _kCompr.concatenate(_lCompr);
            _kCompr.concatenate(_mCompr);
        }
        else if (_nGroups == 1 && _nExtents > 0)
        {
            // length of the single C/P group packed with k-s
            _kTable.push_back(_compressors[0].byteCount());
            _kk = _kCompr.minimize(_kTable, 3);
        }
    }

    void writeOut(std::fstream &out)
    {
        if (_nExtents == 0)
        {
            if (_nGroups > 1)
            {
                unsigned char byte = static_cast<unsigned char>((0x80 | _kk));
                out.write( (const char*)&byte, 1 );
                HCDBG(std::cerr << "writeOut of " << int(byte) << std::endl);
                _kCompr.write(out); // concatenated k,l,m
                for (size_t j = 0; j < _freeComp; j++)
                    _compressors[j].write(out);
            }
            else // single group, no extents; code: 00
            {
                unsigned char k1 = (unsigned char)(_kTable[0]);
                unsigned char k2 = (unsigned char)(_kTable[1]);
                out.write( (const char*)&k1, 1 );
                out.write( (const char*)&k2, 1 );
                _compressors[0].write(out); // C/P
            }
        }
        else
        {               // extents
            unsigned char byte = static_cast<unsigned char>(
                (_nGroups > 1 ? 0xC0 : 0x40) | _kk);
            out.write( (const char*)&byte, 1 );
            _kCompr.write(out);
            for (size_t j = 0; j < _freeComp; j++)
                _compressors[j].write(out);
        }
    }

    Compressor* nextCompressor()
    {
        if (_freeComp == _compressors.size())
            _compressors.push_back(Compressor());
        return _currentCompressor = &_compressors[_freeComp++];
    }

    int byteCount()
    {
        if (_nGroups == 1 && _nExtents == 0)
            return 2 + _compressors[0].byteCount();
        else
        {
            int result = 1;     // initial kk
            result += _kCompr.byteCount();
            for (size_t j = 0; j < _freeComp; j++)
                result += _compressors[j].byteCount();
            return result;
        }
    }
};

int DocumentCompressor::NConceptsInGroup = 16;
int DocumentCompressor::BitsInLabel = 4;
int DocumentCompressor::DefaultSize = 32;

DocumentCompressor& Index::getDocumentCompressor()
{
    if (!_documentCompressor)
        _documentCompressor = new DocumentCompressor();
    return *_documentCompressor;
}

void Index::compress(int docID, int titleID,
        std::vector<ConceptLocation> &locations,
        std::vector<ConceptLocation> &extents)
{
    std::fstream &positions = getPositionsFile();

    positions.seekg(0, std::ios::end);
    long currentEnd = positions.tellg();
    if (currentEnd < 0) currentEnd = 0;
    positions.clear();
    positions.seekg(currentEnd, std::ios::beg);

    _documents.push_back(docID);
    _microIndexOffsets.push_back(currentEnd);
    HCDBG(std::cerr << "_microIndexOffsets pushed back " << currentEnd << std::endl);
    HCDBG(std::cerr << "added title id of " << titleID << std::endl);
    _titles.push_back(titleID);

    getDocumentCompressor().writeOutMicroIndex(positions,
        locations, extents);
}

void Index::writeOutOffsets()
{
    Compressor documents;
    int k1 = documents.minimize(_documents, 8);
    Compressor offsets;
    int k2 = offsets.compressAscending(_microIndexOffsets);
    Compressor titles;
    int k3 = titles.minimize(_titles, 8); // 8 is the starting k
    std::fstream &out = getOffsetsFile();
    out.seekp(0);        // position at beginning
    out.clear();
    unsigned char byte;
    byte = static_cast<unsigned char>(k1);
    out.write( (const char*)&byte, 1 );
    HCDBG(fprintf(stderr, "a: offset dump of %x\n", byte));
    documents.write(out);
    byte = static_cast<unsigned char>(k2);
    out.write( (const char*)&byte, 1 );
    HCDBG(fprintf(stderr, "b: offset dump of %x\n", byte));
    offsets.write(out);
    byte = static_cast<unsigned char>(k3);
    out.write( (const char*)&byte, 1 );
    HCDBG(fprintf(stderr, "c: offset dump of %x\n", byte));
    titles.write(out);
}

Index::~Index()
{
    delete _schema;
    delete _dictParams;
    delete _dict;
    delete _positionsFile;
    delete _offsetsFile;
    delete _documentCompressor;
}

void XmlIndex::compress(int docID, int titleID,
        std::vector<ConceptLocation> &locations,
        std::vector<ConceptLocation> &extents,
        int k, const Compressor &contextTables)
{
    HCDBG(std::cerr << "start compress" << std::endl);
    HCDBG(std::cerr << "docID : " << docID << " titleID : " << titleID <<
        "locations size : " << locations.size() << "extents size : " << extents.size() << std::endl);
    Index::compress(docID, titleID, locations, extents);
    HCDBG(std::cerr << "end compress" << std::endl);

    std::fstream& contexts = getContextsFile();

    contexts.seekp(0, std::ios::end);
    long currentEnd = contexts.tellp();
    if (currentEnd < 0) currentEnd = 0;
    contexts.clear();
    contexts.seekp(currentEnd);
    writeByte(contexts, static_cast<unsigned char>(k));
    contextTables.write(contexts);
    _contextsOffsets.push_back(currentEnd);
}

void XmlIndexBuilder::closeDocument(const std::string &title) throw( HelpProcessingException )
{
    if (_currentDocID == 0)
    {
        std::stringstream aStrStream;
        aStrStream << "no document open" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_INTERNAL_ERROR, aStrStream.str() );
    }
    else if (!_indexAdapter._locations.empty())
    {
        IntegerArray kTable;

        Compressor compressor1;
        Compressor compressor2;
        Compressor compressor3;
        Compressor compressor4;

        kTable.push_back(compressor1.compressAscending(_indexAdapter._initialWords));
        kTable.push_back(compressor2.minimize(_indexAdapter._dests, 2));
        kTable.push_back(compressor3.minimize(_indexAdapter._links, 2));
        kTable.push_back(compressor4.minimize(_indexAdapter._seqNumbers, 2));

        Compressor compressor0;
        int k0 = compressor0.minimize(kTable, 4);

        compressor0.concatenate(compressor1);
        compressor0.concatenate(compressor2);
        compressor0.concatenate(compressor3);
        compressor0.concatenate(compressor4);

        std::vector<ConceptLocation> dummy;
        _indexAdapter._index->compress(_currentDocID, intern(title),
            _indexAdapter._locations, dummy, k0, compressor0);
    }
    else
    {
        // System.out.println("no indexable content");
    }
    _indexAdapter._locations.clear();
    _currentDocID = 0; // state: nothing open
}

void XmlIndexBuilder::indexDocument(xmlDocPtr doc, const std::string &docURL, const std::string &title)
{
    HCDBG(std::cerr << "Indexing " << docURL << std::endl);

    xmlNodePtr root = xmlDocGetRootElement(doc);

    openDocument(docURL);

//    xmlDocDump(stdout, doc);
    xmlDocPtr res = xsltApplyStylesheet(_indexingTransform, doc, NULL);

    _indexAdapter.init();

    // start = System.currentTimeMillis();
    root = xmlDocGetRootElement(res);
    if (root)
    {
//        xmlDocDump(stdout, res);
        for (xmlNodePtr test = root; test; test = test->next)
            _indexAdapter.process(test, res);
    }
    xmlFreeDoc(res);

    // System.out.println((System.currentTimeMillis()-start)+" transform");
    // start = System.currentTimeMillis();
    _indexAdapter.finish();
    // System.out.println((System.currentTimeMillis()-start)+" finish");
    // start = System.currentTimeMillis();
    closeDocument(title);
    // System.out.println((System.currentTimeMillis()-start)+" close");
}

XmlIndexBuilder::~XmlIndexBuilder()
{
    delete _indexAdapter._index;
}

void XmlIndexBuilder::setTransformLocation(const fs::path &filelocation)
{
    _transformLocation = filelocation;
}

xsltStylesheetPtr XmlIndexBuilder::getTransform(const std::string &stylesheetName)
{
    fs::path stylesheet = _transformLocation / (stylesheetName + ".xsl");
    return xsltParseStylesheetFile((const xmlChar *)stylesheet.native_file_string().c_str());
}

void XmlIndexBuilder::initXmlProcessor(const std::string &transform)
{
    _indexingTransform = getTransform(transform);
}

void XmlIndexBuilder::init(const std::string &transform)
{
    _indexAdapter._index->init();
#ifdef EMULATEORIGINAL
    //some kind of bug in the original AFAICS
    _indexAdapter._stoplist.push_back("andnull");
#endif
    reset();

    // initialize vector and hashtable
    const std::vector<std::string> &linkNames = _indexAdapter._index->getLinkNames();
    std::vector<std::string>::const_iterator aEnd = linkNames.end();
    for (std::vector<std::string>::const_iterator aIter = linkNames.begin();
        aIter != aEnd; ++aIter)
    {
        _indexAdapter.getLinkCode(*aIter);
    }

    initXmlProcessor(transform);
}

void XmlIndexBuilder::reset()
{
    _indexAdapter._availContextNumber = 0;
    _indexAdapter._lastWordNumber = 0;
    _indexAdapter._locations.clear();
    _indexAdapter._anyLocationsStored = false;
    // all the contexts' tables
    _indexAdapter._initialWords.clear();
    _indexAdapter._dests.clear();
    _indexAdapter._links.clear();
    _indexAdapter._seqNumbers.clear();
}

XmlIndexBuilder::XmlIndexBuilder(const fs::path &indexDir)
    : _indexingTransform(0), _currentDocID(0)
{
    HCDBG(std::cerr << "indexDir is " << indexDir.native_directory_string() << std::endl);
    _indexAdapter._index = new XmlIndex(indexDir, true);
}

void XmlIndexBuilder::clearIndex()
{
    _indexAdapter._index->clear();
}

class HelpLinker
{
public:
    static void main(std::vector<std::string> &args, std::string* pExtensionPath = NULL )
        throw( HelpProcessingException );
    static bool isExtensionMode( void )
        {return bExtensionMode; }
private:
    HelpLinker() : init(true), xmlIndexBuilder(NULL) {}
    ~HelpLinker() { delete xmlIndexBuilder; }
    JarOutputStream jarOutputStream;
    static int locCount, totCount;
    static Stringtable additionalFiles;
    static HashSet helpFiles;
    static fs::path sourceRoot;
    static fs::path embeddStylesheet;
    static fs::path indexStylesheet;
    static fs::path outputFile;
    static std::string module;
    static std::string lang;
    static std::string hid;
    static std::string extensionPath;
    static bool bExtensionMode;
    fs::path indexDirName;
    Stringtable hidlistTranslation;
    fs::path indexDirParentName;
    bool init;
    XmlIndexBuilder* xmlIndexBuilder;
    void initXMLIndexBuilder();
    void createFileFromBytes(const std::string &fileName,
        const std::string &defaultXSL);
    void closeXMLIndexBuilder()
    {
        xmlIndexBuilder->close();
    }
    void link() throw( HelpProcessingException );
    void addBookmark( DB* dbBase, std::string thishid,
        const std::string& fileB, const std::string& anchorB,
        const std::string& jarfileB, const std::string& titleB );
#if 0
    /**
     * @param outputFile
     * @param module
     * @param lang
     * @param hid
     * @param helpFiles
     * @param additionalFiles
     */

    private HelpURLStreamHandlerFactory urlHandler = null;
#endif
};

bool isExtensionMode( void )
{
    return HelpLinker::isExtensionMode();
}

namespace URLEncoder
{
    static std::string encode(const std::string &rIn)
    {
        const char *good = "!$&'()*+,-.=@_";
        static const char hex[17] = "0123456789ABCDEF";

        std::string result;
        for (size_t i=0; i < rIn.length(); ++i)
        {
            unsigned char c = rIn[i];
            if (isalnum (c) || strchr (good, c))
                result += c;
            else {
                result += '%';
                result += hex[c >> 4];
                result += hex[c & 0xf];
            }
        }
        return result;
    }
}

JarOutputStream::JarOutputStream()
{
    perlline << "use Archive::Zip qw(:ERROR_CODES); ";
    perlline << "my $zip = Archive::Zip->new(); ";
}

std::string replaceAll(std::string result,
    const std::string &search, const std::string &replace)
{
    std::string::size_type pos = 0;
    while(1)
    {
        pos = result.find(search, pos);
        if (pos == std::string::npos) break;
        result.replace(pos, search.size(), replace);
        pos += replace.size();
    }
    return result;
}

void JarOutputStream::addFile(const std::string &fileName, const std::string &name)
{
    perlline << "$zip->addFile(\"" << replaceAll(fileName, "\\", "/") << "\", \"" << name << "\"); ";
}

void JarOutputStream::addTree(const std::string &tree, const std::string &name)
{
    perlline << "$zip->addTree(\"" << replaceAll(tree, "\\", "/") << "\", \"" << name << "\"); ";
}

void JarOutputStream::dontCompress(const std::string &key)
{
    perlline << "my $member = $zip->memberNamed(\"" << key << "\"); ";
    perlline << "if ($member) { $member->desiredCompressionMethod( COMPRESSION_STORED ); } ";
}

void JarOutputStream::commit()
{
    perlline << "print $zip->writeToFileNamed(\"" << replaceAll(getname().native_file_string(), "\\", "/") << "\").\"\\n\"; ";

    fs::path tmp = getname();
    tmp.append(".perl");
    std::string perlfile = replaceAll( tmp.native_file_string(), "\\", "/");
    std::ofstream fos(perlfile.c_str());
    fos << perlline.str();
    fos.close();
    std::string myperl("perl");
    std::string is4nt;
    char* use_shell = getenv( "USE_SHELL" );
    if ( use_shell )
        is4nt = use_shell;
    if( !is4nt.empty() && is4nt == "4nt" )
    {
        // in SO windows environment perl isn't in the path and
        // needs to be fetched from the environment. this doesn't
        // work in a cygwin shell as "/usr/bin/perl" will fail in a
        // native shell (see system call).
        myperl = getenv( "PERL" );
    }
    std::string commandline;
    commandline = myperl + " " + perlfile;
    HCDBG(std::cerr << "command line 3 is" << commandline << std::endl);
    // on windows, calling perl (either cygwin or native) from a native
    // shell the only chance to survive is using "c:/foo" notation
    if ( system(commandline.c_str()) )
        fprintf (stderr, "ERROR: calling generated perl script failed!\n");

    fs::remove(tmp);
}

void HelpLinker::addBookmark( DB* dbBase, std::string thishid,
        const std::string& fileB, const std::string& anchorB,
        const std::string& jarfileB, const std::string& titleB)
{
    HCDBG(std::cerr << "HelpLinker::addBookmark " << thishid << " " <<
        fileB << " " << anchorB << " " << jarfileB << " " << titleB << std::endl);

    std::string temp = thishid;
    std::transform (temp.begin(), temp.end(), temp.begin(), toupper);
    std::replace(temp.begin(), temp.end(), ':', '_');
    const std::string& translatedHid = hidlistTranslation[temp];
    if (!translatedHid.empty())
        thishid = translatedHid;

    thishid = URLEncoder::encode(thishid);

    DBT key;
    memset(&key, 0, sizeof(key));
    key.data = const_cast<char*>(thishid.c_str());
    key.size = thishid.length();

    int fileLen = fileB.length();
    if (!anchorB.empty())
        fileLen += (1 + anchorB.length());
    int dataLen = 1 + fileLen + 1 + jarfileB.length() + 1 + titleB.length();

    std::vector<unsigned char> dataB(dataLen);
    size_t i = 0;
    dataB[i++] = static_cast<unsigned char>(fileLen);
    for (size_t j = 0; j < fileB.length(); ++j)
        dataB[i++] = fileB[j];
    if (!anchorB.empty())
    {
        dataB[i++] = '#';
        for (size_t j = 0; j < anchorB.length(); ++j)
            dataB[i++] = anchorB[j];
    }
    dataB[i++] = static_cast<unsigned char>(jarfileB.length());
    for (size_t j = 0; j < jarfileB.length(); ++j)
        dataB[i++] = jarfileB[j];

    dataB[i++] = static_cast<unsigned char>(titleB.length());
    for (size_t j = 0; j < titleB.length(); ++j)
        dataB[i++] = titleB[j];

    DBT data;
    memset(&data, 0, sizeof(data));
    data.data = &dataB[0];
    data.size = dataB.size();

    dbBase->put(dbBase, NULL, &key, &data, 0);
}

void HelpLinker::createFileFromBytes(const std::string &fileName,
        const std::string &defaultXSL)
{
    std::ofstream fos((indexDirParentName / fileName).native_file_string().c_str());
    fos << defaultXSL;
}

void HelpLinker::initXMLIndexBuilder()
{
    std::string mod = module;
    std::transform (mod.begin(), mod.end(), mod.begin(), tolower);
    indexDirName = indexDirParentName / (mod + ".idx");
    fs::create_directory(indexDirName);

    if (xmlIndexBuilder) delete xmlIndexBuilder;
    xmlIndexBuilder = new XmlIndexBuilder(indexDirName);

    std::string defaultXSL =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\">\n"
            "\t<xsl:template match=\"*|/\"/>\n"
            "</xsl:stylesheet>";
    createFileFromBytes("default.xsl", defaultXSL);
    xmlIndexBuilder->clearIndex(); // Build index from scratch
    xmlIndexBuilder->setTransformLocation(indexDirParentName);
}

namespace
{
    fs::path gettmppath()
    {
        fs::path ret;
        osl::File::createTempFile(0, 0, &ret.data);
        fs::remove(ret);
        return ret;
    }
}

extern "C" void function_orig_pointer(xmlXPathParserContextPtr ctxt, int nargs)
{
    if (nargs > 1)
    {
        // TODO: Change when used for extensions, no exception possible here
        std::cerr << "function_orig_pointer, too many args" << std::endl;
        exit(-1);
    }

    xmlNodePtr cur = NULL;
    if (nargs == 0)
        cur = ctxt->context->node;
    else if (nargs == 1)
    {
        xmlXPathObjectPtr obj = valuePop(ctxt);
        xmlNodeSetPtr nodelist = obj->nodesetval;

        if ((nodelist == NULL) || (nodelist->nodeNr <= 0))
        {
            // TODO: Change when used for extensions, no exception possible here
            std::cerr << "function_orig_pointer, bad nodeset" << std::endl;
            exit(-1);
        }

        cur = nodelist->nodeTab[0];
        for (int i = 1; i < nodelist->nodeNr; ++i)
        {
            int ret = xmlXPathCmpNodes(cur, nodelist->nodeTab[i]);
            if (ret == -1)
                cur = nodelist->nodeTab[i];
        }

        xmlXPathFreeObject(obj);
    }

    if (cur == NULL)
    {
        // TODO: Change when used for extensions, no exception possible here
        std::cerr << "function_orig_pointer, bad node" << std::endl;
        exit(-1);
    }

    static xmlChar str[20];
    sprintf((char *)str, "%ld", (sal_uIntPtr)(cur));
    valuePush(ctxt, xmlXPathNewString(str));
}

extern "C" void* cmc_module_init(xsltTransformContextPtr ctxt, const xmlChar* uri)
{
    if (xsltRegisterExtFunction(ctxt, (const xmlChar*)"orig-pointer", uri, function_orig_pointer))
    {
        // TODO: Change when used for extensions, no exception possible here
        std::cerr << "failure to register function_orig_pointer" << std::endl;
        exit(-1);
    }
    return NULL;
}

extern "C" void cmc_module_term(xsltTransformContextPtr, const xmlChar*, void*)
{
}

/**
*
*/
void HelpLinker::link() throw( HelpProcessingException )
{
    bool bIndexForExtension = false;        // TODO

    if( bExtensionMode )
    {
        indexDirParentName = sourceRoot;
    }
    else
    {
        indexDirParentName = gettmppath();
        fs::create_directory(indexDirParentName);
    }

#ifdef CMC_DEBUG
    std::cerr << "will not delete tmpdir of " << indexDirParentName.native_file_string().c_str() << std::endl;
#endif

    std::string mod = module;
    std::transform (mod.begin(), mod.end(), mod.begin(), tolower);

    // Determine the outputstream
    fs::path outputTmpFile;
    if( !bExtensionMode )
    {
        outputTmpFile = outputFile;
        outputTmpFile.append(".tmp");
        jarOutputStream.setname(outputTmpFile);
    }

    // do the work here
    // continue with introduction of the overall process thing into the
    // here all hzip files will be worked on
    std::string appl = mod;
    if (appl[0] == 's')
        appl = appl.substr(1);

    fs::path helpTextFileName(indexDirParentName / (mod + ".ht"));
    DB* helpText(0);
    db_create(&helpText,0,0);
    helpText->open(helpText, NULL, helpTextFileName.native_file_string().c_str(), NULL, DB_BTREE,
        DB_CREATE | DB_TRUNCATE, 0644);

    fs::path dbBaseFileName(indexDirParentName / (mod + ".db"));
    DB* dbBase(0);
    db_create(&dbBase,0,0);
    dbBase->open(dbBase, NULL, dbBaseFileName.native_file_string().c_str(), NULL, DB_BTREE,
        DB_CREATE | DB_TRUNCATE, 0644);

    fs::path keyWordFileName(indexDirParentName / (mod + ".key"));
    DB* keyWord(0);
    db_create(&keyWord,0,0);
    keyWord->open(keyWord, NULL, keyWordFileName.native_file_string().c_str(), NULL, DB_BTREE,
        DB_CREATE | DB_TRUNCATE, 0644);

    HelpKeyword helpKeyword;

    // catch HelpProcessingException to avoid locking data bases
    try
    {

    std::ifstream fileReader(hid.c_str());
    while (fileReader)
    {
        std::string key;
        fileReader >> key;
        std::transform (key.begin(), key.end(), key.begin(), toupper);
        std::replace(key.begin(), key.end(), ':', '_');
        std::string data;
        fileReader >> data;
        if (!key.empty() && !data.empty())
            hidlistTranslation[key] = data;
    }

    // lastly, initialize the indexBuilder
    if ( (!bExtensionMode || bIndexForExtension) && !helpFiles.empty())
        initXMLIndexBuilder();

    if( !bExtensionMode )
    {
        std::cout << "Making " << outputFile.native_file_string() <<
            " from " << helpFiles.size() << " input files" << std::endl;
    }

    // here we start our loop over the hzip files.
    HashSet::iterator end = helpFiles.end();
    for (HashSet::iterator iter = helpFiles.begin(); iter != end; ++iter)
    {
        std::cout << ".";
        std::cout.flush();
        // process one file
        // streamTable contains the streams in the hzip file
        StreamTable streamTable;
        const std::string &xhpFileName = *iter;

        if (!bExtensionMode && xhpFileName.rfind(".xhp") != xhpFileName.length()-4)
        {
            // only work on .xhp - files
            std::cerr <<
                "ERROR: input list entry '"
                    << xhpFileName
                    << "' has the wrong extension (only files with extension .xhp "
                    << "are accepted)";
            continue;
        }

        fs::path langsourceRoot(sourceRoot);
        fs::path xhpFile;
        if( bExtensionMode )
        {
            // langsourceRoot == sourceRoot for extensions
            std::string xhpFileNameComplete( extensionPath );
            xhpFileNameComplete.append( '/' + xhpFileName );
            xhpFile = fs::path( xhpFileNameComplete );
        }
        else
        {
            langsourceRoot.append('/' + lang + '/');
            xhpFile = fs::path(xhpFileName, fs::native);
        }
        HelpCompiler hc( streamTable, xhpFile, langsourceRoot,
            embeddStylesheet, module, lang, bExtensionMode );

        HCDBG(std::cerr << "before compile of " << xhpFileName << std::endl);
        bool success = hc.compile();
        HCDBG(std::cerr << "after compile of " << xhpFileName << std::endl);

        if (!success && !bExtensionMode)
        {
            std::stringstream aStrStream;
            aStrStream <<
                "\nERROR: compiling help particle '"
                    << xhpFileName
                    << "' for language '"
                    << lang
                    << "' failed!";
            throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
        }

        const std::string documentBaseId = streamTable.document_id;
        std::string documentPath = streamTable.document_path;
        if (documentPath.find("/") == 0)
            documentPath = documentPath.substr(1);

        std::string documentJarfile = streamTable.document_module + ".jar";

        std::string documentTitle = streamTable.document_title;
        if (documentTitle.empty())
            documentTitle = "<notitle>";

#if 0
        std::cout << "for " << xhpFileName << " documentBaseId is " << documentBaseId << "\n";
        std::cout << "for " << xhpFileName << " documentPath is " << documentPath << "\n";
        std::cout << "for " << xhpFileName << " documentJarfile is " << documentJarfile << "\n";
        std::cout << "for " << xhpFileName << " documentPath is " << documentTitle << "\n";
#endif

        const std::string& fileB = documentPath;
        const std::string& jarfileB = documentJarfile;
        std::string& titleB = documentTitle;

        // add once this as its own id.
        addBookmark(dbBase, documentPath, fileB, std::string(), jarfileB, titleB);

        if ( (!bExtensionMode || bIndexForExtension) && init)
        {
            std::ifstream indexXSLFile(indexStylesheet.native_file_string().c_str());
            std::ostringstream baos;
            baos << indexXSLFile.rdbuf();
            std::string xsl = baos.str();

            //I see that we later generate a map of generateids to nodes which we will use
            //to link the results of generate-id in the transformed document back to the nodes
            //in the original document, so let's cut out the middle-men and make an extension
            //which does exactly what we want, and give us a pointer to the original node
            xsl.replace(xsl.find("<xsl:stylesheet"), strlen("<xsl:stylesheet"),
                "<xsl:stylesheet extension-element-prefixes=\"CMC\" xmlns:CMC=\"http://www.cunninghack.org\"");
            xsl.replace(xsl.find("generate-id"), strlen("generate-id"), "CMC:orig-pointer");

            if (xsltRegisterExtModule((const xmlChar*)"http://www.cunninghack.org", cmc_module_init, cmc_module_term))
            {
                std::stringstream aStrStream;
                aStrStream << "fatal error on registering xslt module" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_INTERNAL_ERROR, aStrStream.str() );
            }

            createFileFromBytes("index.xsl", xsl);
            xmlIndexBuilder->init("index");
            init = false;
        }

        // first the database *.db
        // ByteArrayInputStream bais = null;
        // ObjectInputStream ois = null;

        const HashSet *hidlist = streamTable.appl_hidlist;
        if (!hidlist)
            hidlist = streamTable.default_hidlist;
        if (hidlist && !hidlist->empty())
        {
            // now iterate over all elements of the hidlist
            HashSet::const_iterator aEnd = hidlist->end();
            for (HashSet::const_iterator hidListIter = hidlist->begin();
                hidListIter != aEnd; ++hidListIter)
            {
                std::string thishid = *hidListIter;

                std::string anchorB;
                size_t index = thishid.rfind('#');
                if (index != std::string::npos)
                {
                    anchorB = thishid.substr(1 + index);
                    thishid = thishid.substr(0, index);
                }
                addBookmark(dbBase, thishid, fileB, anchorB, jarfileB, titleB);
            }
        }

        // now the keywords
        const Hashtable *anchorToLL = streamTable.appl_keywords;
        if (!anchorToLL)
            anchorToLL = streamTable.default_keywords;
        if (anchorToLL && !anchorToLL->empty())
        {
            std::string fakedHid = URLEncoder::encode(documentPath);
            Hashtable::const_iterator aEnd = anchorToLL->end();
            for (Hashtable::const_iterator enumer = anchorToLL->begin();
                enumer != aEnd; ++enumer)
            {
                const std::string &anchor = enumer->first;
                addBookmark(dbBase, documentPath, fileB,
                    anchor, jarfileB, titleB);
                std::string totalId = fakedHid + "#" + anchor;
                // std::cerr << hzipFileName << std::endl;
                const LinkedList& ll = enumer->second;
                LinkedList::const_iterator aOtherEnd = ll.end();
                for (LinkedList::const_iterator llIter = ll.begin();
                    llIter != aOtherEnd; ++llIter)
                {
                        helpKeyword.insert(*llIter, totalId);
                }
            }

        }

        // and last the helptexts
        const Stringtable *helpTextHash = streamTable.appl_helptexts;
        if (!helpTextHash)
            helpTextHash = streamTable.default_helptexts;
        if (helpTextHash && !helpTextHash->empty())
        {
            Stringtable::const_iterator aEnd = helpTextHash->end();
            for (Stringtable::const_iterator helpTextIter = helpTextHash->begin();
                helpTextIter != aEnd; ++helpTextIter)
            {
                std::string helpTextId = helpTextIter->first;
                const std::string& helpTextText = helpTextIter->second;

                std::string temp = helpTextId;
                std::transform (temp.begin(), temp.end(), temp.begin(), toupper);
                std::replace(temp.begin(), temp.end(), ':', '_');

                const std::string& tHid = hidlistTranslation[temp];
                if (!tHid.empty())
                    helpTextId = tHid;
                helpTextId = URLEncoder::encode(helpTextId);

                DBT keyDbt;
                memset(&keyDbt, 0, sizeof(keyDbt));
                keyDbt.data = const_cast<char*>(helpTextId.c_str());
                keyDbt.size = helpTextId.length();

                DBT textDbt;
                memset(&textDbt, 0, sizeof(textDbt));
                textDbt.data = const_cast<char*>(helpTextText.c_str());
                textDbt.size = helpTextText.length();
                helpText->put(helpText, NULL, &keyDbt, &textDbt, 0);
            }
        }

        if( !bExtensionMode || bIndexForExtension )
        {
            // now the indexing
            xmlDocPtr document = streamTable.appl_doc;
            if (!document)
                document = streamTable.default_doc;
            if (document)
            {
                std::string temp = module;
                std::transform (temp.begin(), temp.end(), temp.begin(), tolower);
                xmlIndexBuilder->indexDocument(document,
                        std::string("vnd.sun.star.help://")
                                + temp
                                + "/"
                                + URLEncoder::encode(documentPath),
                                "");
            }
        }
    } // while loop over hzip files ending

    if( !bExtensionMode )
        std::cout << std::endl;

    }   // try
    catch( HelpProcessingException& )
    {
        // catch HelpProcessingException to avoid locking data bases
        helpText->close(helpText, 0);
        dbBase->close(dbBase, 0);
        keyWord->close(keyWord, 0);
        throw;
    }

    helpText->close(helpText, 0);
    dbBase->close(dbBase, 0);
    helpKeyword.dump(keyWord);
    keyWord->close(keyWord, 0);

    if (!bExtensionMode && !helpFiles.empty())
    {
        closeXMLIndexBuilder();
        HCDBG(std::cerr << "dir is " << indexDirName.native_directory_string() << std::endl);
        jarOutputStream.addTree(indexDirName.native_file_string(), mod + ".idx");
    }

    if( !bExtensionMode )
    {
        jarOutputStream.addFile(helpTextFileName.native_file_string(), mod + ".ht");
        jarOutputStream.addFile(dbBaseFileName.native_file_string(), mod + ".db");
        jarOutputStream.addFile(keyWordFileName.native_file_string(), mod + ".key");

        /////////////////////////////////////////////////////////////////////////
        // last, all files which should be copied into the jar file
        /////////////////////////////////////////////////////////////////////////

        Stringtable::iterator aEnd = additionalFiles.end();
        for (Stringtable::iterator enumer = additionalFiles.begin(); enumer != aEnd;
            ++enumer)
        {
            const std::string &additionalFileKey = enumer->first;
            const std::string &additionalFileName = enumer->second;
            jarOutputStream.addFile(additionalFileName, additionalFileKey);
        }

        jarOutputStream.dontCompress(mod + ".jar");
        jarOutputStream.commit();

        HCDBG(std::cerr << "like to rename " << outputTmpFile.native_file_string() << " as " <<
            outputFile.native_file_string() << std::endl);
        fs::rename(outputTmpFile, outputFile);
        if (!fs::exists(outputFile))
        {
            std::stringstream aStrStream;
            aStrStream << "can't rename file '" << outputTmpFile.native_file_string() << "'" << std::endl;
            throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
        }
    }

    /////////////////////////////////////////////////////////////////////////
    /// remove temprary directory for index creation
    /////////////////////////////////////////////////////////////////////////
#ifndef CMC_DEBUG
    if( !bExtensionMode )
        fs::remove_all( indexDirParentName );
#endif
}


int HelpLinker::locCount;
int HelpLinker::totCount;
Stringtable HelpLinker::additionalFiles;
HashSet HelpLinker::helpFiles;
fs::path HelpLinker::sourceRoot;
fs::path HelpLinker::embeddStylesheet, HelpLinker::indexStylesheet;
fs::path HelpLinker::outputFile;
std::string HelpLinker::module;
std::string HelpLinker::lang;
std::string HelpLinker::hid;
std::string HelpLinker::extensionPath;
bool HelpLinker::bExtensionMode;

int GnTmpFileCounter = 0;

void HelpLinker::main(std::vector<std::string> &args, std::string* pExtensionPath)
    throw( HelpProcessingException )
{
    bExtensionMode = false;
    if( pExtensionPath && pExtensionPath->length() > 0 )
    {
        helpFiles.clear();
        bExtensionMode = true;
        extensionPath = *pExtensionPath;
        sourceRoot = fs::path(extensionPath);
    }
    if (args.size() > 0 && args[0][0] == '@')
    {
        std::vector<std::string> stringList;
        std::string strBuf;
        std::ifstream fileReader(args[0].substr(1).c_str());

        while (fileReader)
        {
            std::string token;
            fileReader >> token;
            if (!token.empty())
                stringList.push_back(token);
        }

        args = stringList;
    }

    size_t i = 0;

    while (i < args.size())
    {
        if (args[i].compare("-src") == 0)
        {
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "sourceroot missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }

            if( !bExtensionMode )
                sourceRoot = fs::path(args[i], fs::native);
        }
        else if (args[i].compare("-sty") == 0)
        {
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "embeddingStylesheet missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }

            embeddStylesheet = fs::path(args[i], fs::native);
        }
        else if (args[i].compare("-idx") == 0)
        {
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "indexstylesheet missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }

            indexStylesheet = fs::path(args[i], fs::native);
        }
        else if (args[i].compare("-o") == 0)
        {
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "outputfilename missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }

            outputFile = fs::path(args[i], fs::native);
        }
        else if (args[i].compare("-mod") == 0)
        {
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "module name missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }

            module = args[i];
        }
        else if (args[i].compare("-lang") == 0)
        {
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "language name missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }

            lang = args[i];
        }
        else if (args[i].compare("-hid") == 0)
        {
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "hid list missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }

            hid = args[i];
        }
        else if (args[i].compare("-add") == 0)
        {
            std::string addFile, addFileUnderPath;
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "pathname missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }

            addFileUnderPath = args[i];
            ++i;
            if (i >= args.size())
            {
                std::stringstream aStrStream;
                aStrStream << "pathname missing" << std::endl;
                throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
            }
            addFile = args[i];
            if (!addFileUnderPath.empty() && !addFile.empty())
                additionalFiles[addFileUnderPath] = addFile;
        }
        else
            helpFiles.push_back(args[i]);
        ++i;
    }

    if (!bExtensionMode && indexStylesheet.empty())
    {
        std::stringstream aStrStream;
        aStrStream << "no index file given" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
    }
    if (!bExtensionMode && embeddStylesheet.empty())
    {
        std::stringstream aStrStream;
        aStrStream << "no embedding resolving file given" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
    }
    if (sourceRoot.empty())
    {
        std::stringstream aStrStream;
        aStrStream << "no sourceroot given" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
    }
    if (!bExtensionMode && outputFile.empty())
    {
        std::stringstream aStrStream;
        aStrStream << "no output file given" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
    }
    if (module.empty())
    {
        std::stringstream aStrStream;
        aStrStream << "module missing" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
    }
    if (!bExtensionMode && lang.empty())
    {
        std::stringstream aStrStream;
        aStrStream << "language missing" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
    }
    if (!bExtensionMode && hid.empty())
    {
        std::stringstream aStrStream;
        aStrStream << "hid list missing" << std::endl;
        throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
    }

    HelpLinker().link();
}

int main(int argc, char**argv)
{
    sal_uInt32 starttime = osl_getGlobalTimer();
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i)
        args.push_back(std::string(argv[i]));
    try
    {
        HelpLinker::main(args);
    }
    catch( const HelpProcessingException& e )
    {
        std::cerr << e.m_aErrorMsg;
        exit(1);
    }
    sal_uInt32 endtime = osl_getGlobalTimer();
    std::cout << "time taken was " << (endtime-starttime)/1000.0 << " seconds" << std::endl;
    return 0;
}

// Variable to set an exception in "C" StructuredXMLErrorFunction
static const HelpProcessingException* GpXMLParsingException = NULL;

extern "C" void StructuredXMLErrorFunction(void *userData, xmlErrorPtr error)
{
    (void)userData;
    (void)error;

    std::string aErrorMsg = error->message;
    std::string aXMLParsingFile;
    if( error->file != NULL )
        aXMLParsingFile = error->file;
    int nXMLParsingLine = error->line;
    HelpProcessingException* pException = new HelpProcessingException( aErrorMsg, aXMLParsingFile, nXMLParsingLine );
    GpXMLParsingException = pException;

    // Reset error handler
    xmlSetStructuredErrorFunc( NULL, NULL );
}

HelpProcessingErrorInfo& HelpProcessingErrorInfo::operator=( const struct HelpProcessingException& e )
{
    m_eErrorClass = e.m_eErrorClass;
    rtl::OString tmpErrorMsg( e.m_aErrorMsg.c_str() );
    m_aErrorMsg = rtl::OStringToOUString( tmpErrorMsg, osl_getThreadTextEncoding() );
    rtl::OString tmpXMLParsingFile( e.m_aXMLParsingFile.c_str() );
    m_aXMLParsingFile = rtl::OStringToOUString( tmpXMLParsingFile, osl_getThreadTextEncoding() );
    m_nXMLParsingLine = e.m_nXMLParsingLine;
    return *this;
}

// Returns true in case of success, false in case of error
HELPLINKER_DLLPUBLIC bool compileExtensionHelp
(
    const rtl::OUString& aExtensionName,
    const rtl::OUString& aExtensionLanguageRoot,
    sal_Int32 nXhpFileCount, const rtl::OUString* pXhpFiles,
    HelpProcessingErrorInfo& o_rHelpProcessingErrorInfo
)
{
    bool bSuccess = true;

    sal_uInt32 starttime = osl_getGlobalTimer();

    sal_Int32 argc = nXhpFileCount + 3;
    const char** argv = new const char*[argc];
    argv[0] = "";
    argv[1] = "-mod";
    rtl::OString aOExtensionName = rtl::OUStringToOString( aExtensionName, osl_getThreadTextEncoding() );
    argv[2] = aOExtensionName.getStr();

    for( sal_Int32 iXhp = 0 ; iXhp < nXhpFileCount ; ++iXhp )
    {
        rtl::OUString aXhpFile = pXhpFiles[iXhp];

        rtl::OString aOXhpFile = rtl::OUStringToOString( aXhpFile, osl_getThreadTextEncoding() );
        char* pArgStr = new char[aOXhpFile.getLength() + 1];
        strcpy( pArgStr, aOXhpFile.getStr() );
        argv[iXhp + 3] = pArgStr;
    }

    std::vector<std::string> args;
    for( sal_Int32 i = 1; i < argc; ++i )
        args.push_back(std::string( argv[i]) );

    for( sal_Int32 iXhp = 0 ; iXhp < nXhpFileCount ; ++iXhp )
        delete argv[iXhp + 3];
    delete[] argv;

    rtl::OString aOExtensionLanguageRoot = rtl::OUStringToOString( aExtensionLanguageRoot, osl_getThreadTextEncoding() );
    const char* pExtensionPath = aOExtensionLanguageRoot.getStr();
    std::string aStdStrExtensionPath = pExtensionPath;

    // Set error handler
    xmlSetStructuredErrorFunc( NULL, (xmlStructuredErrorFunc)StructuredXMLErrorFunction );
    try
    {
        HelpLinker::main(args,&aStdStrExtensionPath);
    }
    catch( const HelpProcessingException& e )
    {
        if( GpXMLParsingException != NULL )
        {
            o_rHelpProcessingErrorInfo = *GpXMLParsingException;
            delete GpXMLParsingException;
            GpXMLParsingException = NULL;
        }
        else
        {
            o_rHelpProcessingErrorInfo = e;
        }
        bSuccess = false;
    }
    // Reset error handler
    xmlSetStructuredErrorFunc( NULL, NULL );

    sal_uInt32 endtime = osl_getGlobalTimer();
    double dTimeInSeconds = (endtime-starttime) / 1000.0;
    (void)dTimeInSeconds;

    return bSuccess;
}


// vnd.sun.star.help://swriter/52821?Language=en-US&System=UNIX
/* vi:set tabstop=4 shiftwidth=4 expandtab: */
